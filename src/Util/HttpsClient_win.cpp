//
//  HttpsClient_win.cpp
//
//  httplib::SSLClient over WinHTTP (see HttpsClient.hpp). TLS and
//  certificate verification are Windows' own, against the system store.
//

#if defined(_WIN32) && !defined(CPPHTTPLIB_OPENSSL_SUPPORT)

#include "HttpsClient.hpp"

#include <windows.h>
#include <winhttp.h>

#include <random>
#include <sstream>
#include <vector>

#pragma comment(lib, "winhttp.lib")

namespace httplib {

namespace {

std::wstring widen(const std::string& s) {
  if (s.empty()) return {};
  const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), nullptr, 0);
  std::wstring out(n, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), out.data(), n);
  return out;
}

std::string narrow(const wchar_t* s, size_t len) {
  if (!len) return {};
  const int n = WideCharToMultiByte(CP_UTF8, 0, s, int(len), nullptr, 0, nullptr, nullptr);
  std::string out(n, '\0');
  WideCharToMultiByte(CP_UTF8, 0, s, int(len), out.data(), n, nullptr, nullptr);
  return out;
}

Error errorFor(DWORD code) {
  switch (code) {
  case ERROR_WINHTTP_TIMEOUT:                       return Error::ConnectionTimeout;
  case ERROR_WINHTTP_NAME_NOT_RESOLVED:
  case ERROR_WINHTTP_CANNOT_CONNECT:
  case ERROR_WINHTTP_CONNECTION_ERROR:              return Error::Connection;
  case ERROR_WINHTTP_SECURE_FAILURE:
  case ERROR_WINHTTP_SECURE_CERT_DATE_INVALID:
  case ERROR_WINHTTP_SECURE_CERT_CN_INVALID:
  case ERROR_WINHTTP_SECURE_INVALID_CA:
  case ERROR_WINHTTP_SECURE_CERT_REV_FAILED:
  case ERROR_WINHTTP_SECURE_CERT_REVOKED:
  case ERROR_WINHTTP_SECURE_INVALID_CERT:           return Error::SSLServerVerification;
  case ERROR_WINHTTP_REDIRECT_FAILED:               return Error::ExceedRedirectCount;
  case ERROR_WINHTTP_OPERATION_CANCELLED:           return Error::Canceled;
  default:                                          return Error::Unknown;
  }
}

// Closes a WinHTTP handle when it goes out of scope.
struct Handle {
  HINTERNET h = nullptr;
  explicit Handle(HINTERNET handle) : h(handle) {}
  ~Handle() { if (h) WinHttpCloseHandle(h); }
  Handle(const Handle&) = delete;
  Handle& operator=(const Handle&) = delete;
  explicit operator bool() const { return h != nullptr; }
};

// Parses WINHTTP_QUERY_RAW_HEADERS_CRLF output (status line first).
void parseHeaders(const std::string& raw, Headers& out) {
  std::istringstream lines(raw);
  std::string line;
  std::getline(lines, line);  // status line
  while (std::getline(lines, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    const size_t colon = line.find(':');
    if (colon == std::string::npos) continue;
    size_t valueStart = colon + 1;
    while (valueStart < line.size() && line[valueStart] == ' ') ++valueStart;
    out.emplace(line.substr(0, colon), line.substr(valueStart));
  }
}

} // namespace

Result SSLClient::send(const char* method, const std::string& path, const Headers& headers,
                       const std::string& body, const std::string& contentType, Progress progress) {
  // The host may carry a port ("example.com:8443").
  std::string host = host_;
  INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT;
  if (const size_t colon = host.rfind(':'); colon != std::string::npos) {
    port = INTERNET_PORT(std::stoi(host.substr(colon + 1)));
    host.resize(colon);
  }

  Handle session(WinHttpOpen(L"Nottawa/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                             WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));
  if (!session) return Result{nullptr, errorFor(GetLastError())};
  WinHttpSetTimeouts(session.h, connectionTimeoutMs_, connectionTimeoutMs_, writeTimeoutMs_, readTimeoutMs_);
  // Prefer TLS 1.2/1.3 even on older Windows defaults.
  DWORD protocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3;
  if (!WinHttpSetOption(session.h, WINHTTP_OPTION_SECURE_PROTOCOLS, &protocols, sizeof(protocols))) {
    protocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
    WinHttpSetOption(session.h, WINHTTP_OPTION_SECURE_PROTOCOLS, &protocols, sizeof(protocols));
  }

  Handle connection(WinHttpConnect(session.h, widen(host).c_str(), port, 0));
  if (!connection) return Result{nullptr, errorFor(GetLastError())};

  Handle request(WinHttpOpenRequest(connection.h, widen(method).c_str(), widen(path).c_str(), nullptr,
                                    WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE));
  if (!request) return Result{nullptr, errorFor(GetLastError())};
  DWORD redirects = followLocation_ ? WINHTTP_OPTION_REDIRECT_POLICY_DISALLOW_HTTPS_TO_HTTP
                                    : WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
  WinHttpSetOption(request.h, WINHTTP_OPTION_REDIRECT_POLICY, &redirects, sizeof(redirects));

  std::string headerBlock;
  Headers all = defaultHeaders_;
  all.insert(headers.begin(), headers.end());
  for (const auto& [name, value] : all) headerBlock += name + ": " + value + "\r\n";
  if (!contentType.empty()) headerBlock += "Content-Type: " + contentType + "\r\n";
  const std::wstring wideHeaders = widen(headerBlock);

  if (!WinHttpSendRequest(request.h,
                          wideHeaders.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : wideHeaders.c_str(),
                          wideHeaders.empty() ? 0 : DWORD(-1L),
                          body.empty() ? WINHTTP_NO_REQUEST_DATA : const_cast<char*>(body.data()),
                          DWORD(body.size()), DWORD(body.size()), 0)
      || !WinHttpReceiveResponse(request.h, nullptr)) {
    return Result{nullptr, errorFor(GetLastError())};
  }

  auto response = std::make_unique<Response>();
  DWORD status = 0, size = sizeof(status);
  WinHttpQueryHeaders(request.h, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                      WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX);
  response->status = int(status);

  DWORD rawSize = 0;
  WinHttpQueryHeaders(request.h, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX,
                      WINHTTP_NO_OUTPUT_BUFFER, &rawSize, WINHTTP_NO_HEADER_INDEX);
  if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && rawSize > 0) {
    std::vector<wchar_t> raw(rawSize / sizeof(wchar_t) + 1);
    if (WinHttpQueryHeaders(request.h, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX,
                            raw.data(), &rawSize, WINHTTP_NO_HEADER_INDEX)) {
      parseHeaders(narrow(raw.data(), rawSize / sizeof(wchar_t)), response->headers);
    }
  }
  uint64_t total = 0;
  if (response->has_header("Content-Length")) {
    try { total = std::stoull(response->get_header_value("Content-Length")); } catch (...) {}
  }

  if (std::string(method) != "HEAD") {
    std::vector<char> chunk(64 * 1024);
    for (;;) {
      DWORD read = 0;
      if (!WinHttpReadData(request.h, chunk.data(), DWORD(chunk.size()), &read))
        return Result{nullptr, errorFor(GetLastError())};
      if (read == 0) break;
      response->body.append(chunk.data(), read);
      if (progress && !progress(response->body.size(), total)) return Result{nullptr, Error::Canceled};
    }
  }
  return Result{std::move(response), Error::Success};
}

Result SSLClient::Post(const std::string& path, const Headers& headers, const MultipartFormDataItems& items) {
  static std::mt19937_64 rng{std::random_device{}()};
  std::ostringstream boundary;
  boundary << "----NottawaFormBoundary" << std::hex << rng() << rng();
  const std::string b = boundary.str();

  std::string body;
  for (const auto& item : items) {
    body += "--" + b + "\r\n";
    body += "Content-Disposition: form-data; name=\"" + item.name + "\"";
    if (!item.filename.empty()) body += "; filename=\"" + item.filename + "\"";
    body += "\r\n";
    if (!item.content_type.empty()) body += "Content-Type: " + item.content_type + "\r\n";
    body += "\r\n" + item.content + "\r\n";
  }
  body += "--" + b + "--\r\n";
  return send("POST", path, headers, body, "multipart/form-data; boundary=" + b, nullptr);
}

} // namespace httplib

#endif // _WIN32 && !CPPHTTPLIB_OPENSSL_SUPPORT
