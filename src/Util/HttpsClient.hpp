//
//  HttpsClient.hpp
//
//  httplib::SSLClient for every platform. Include this instead of using
//  httplib's SSLClient directly; it defines NOTTAWA_HAS_HTTPS when HTTPS
//  requests can actually be made:
//
//    - With CPPHTTPLIB_OPENSSL_SUPPORT (macOS): httplib's own SSLClient.
//    - On Windows: the same interface over WinHTTP (HttpsClient_win.cpp),
//      so no OpenSSL is needed. Certificates are always verified against
//      the Windows certificate store, whatever
//      enable_server_certificate_verification() is given.
//    - Otherwise: a stub whose requests all fail, so the HTTPS code paths
//      (guarded by NOTTAWA_HAS_HTTPS) still type-check.
//
//  Only the subset of the interface Nottawa uses is provided.
//

#ifndef HttpsClient_hpp
#define HttpsClient_hpp

#include "httplib.h"

#if defined(CPPHTTPLIB_OPENSSL_SUPPORT)

#define NOTTAWA_HAS_HTTPS 1

#else

#if defined(_WIN32)
#define NOTTAWA_HAS_HTTPS 1
#endif

#include <cstdint>
#include <string>

namespace httplib {

class SSLClient {
public:
  explicit SSLClient(const std::string& host) : host_(host) {}
  explicit SSLClient(const char* host) : host_(host ? host : "") {}

  void enable_server_certificate_verification(bool) {}
  void set_connection_timeout(time_t sec, time_t usec = 0) { connectionTimeoutMs_ = toMs(sec, usec); }
  void set_read_timeout(time_t sec, time_t usec = 0)       { readTimeoutMs_ = toMs(sec, usec); }
  void set_write_timeout(time_t sec, time_t usec = 0)      { writeTimeoutMs_ = toMs(sec, usec); }
  void set_follow_location(bool on)                        { followLocation_ = on; }
  void set_default_headers(const Headers& headers)         { defaultHeaders_ = headers; }

  Result Get(const std::string& path)                                     { return Get(path, Headers{}, nullptr); }
  Result Get(const std::string& path, const Headers& headers)             { return Get(path, headers, nullptr); }
  Result Get(const std::string& path, const Headers& headers, Progress progress) {
    return send("GET", path, headers, std::string(), std::string(), std::move(progress));
  }
  Result Get(const char* path)                                            { return Get(std::string(path)); }
  Result Get(const char* path, const Headers& headers)                    { return Get(std::string(path), headers); }
  Result Get(const char* path, const Headers& headers, Progress progress) {
    return Get(std::string(path), headers, std::move(progress));
  }

  Result Post(const std::string& path, const std::string& body, const std::string& contentType) {
    return Post(path, Headers{}, body, contentType);
  }
  Result Post(const std::string& path, const Headers& headers, const std::string& body,
              const std::string& contentType) {
    return send("POST", path, headers, body, contentType, nullptr);
  }
  Result Post(const std::string& path, const Headers& headers, const MultipartFormDataItems& items);
  Result Post(const char* path, const std::string& body, const char* contentType) {
    return Post(std::string(path), body, std::string(contentType ? contentType : ""));
  }
  Result Post(const char* path, const Headers& headers, const std::string& body, const char* contentType) {
    return Post(std::string(path), headers, body, std::string(contentType ? contentType : ""));
  }
  Result Post(const char* path, const Headers& headers, const MultipartFormDataItems& items) {
    return Post(std::string(path), headers, items);
  }

  Result Head(const std::string& path) {
    return send("HEAD", path, Headers{}, std::string(), std::string(), nullptr);
  }
  Result Head(const char* path) { return Head(std::string(path)); }

private:
  static int toMs(time_t sec, time_t usec) { return int(sec * 1000 + usec / 1000); }

  // Performs one request; the body is returned in Response::body.
  Result send(const char* method, const std::string& path, const Headers& headers,
              const std::string& body, const std::string& contentType, Progress progress);

  std::string host_;
  Headers     defaultHeaders_;
  int         connectionTimeoutMs_ = 300 * 1000;
  int         readTimeoutMs_       = 300 * 1000;
  int         writeTimeoutMs_      = 5 * 1000;
  bool        followLocation_      = false;
};

#if !defined(_WIN32)
// No HTTPS backend: every request fails without a response.
inline Result SSLClient::send(const char*, const std::string&, const Headers&, const std::string&,
                              const std::string&, Progress) {
  return Result{nullptr, Error::Unknown};
}
inline Result SSLClient::Post(const std::string&, const Headers&, const MultipartFormDataItems&) {
  return Result{nullptr, Error::Unknown};
}
#endif

} // namespace httplib

#endif // CPPHTTPLIB_OPENSSL_SUPPORT
#endif // HttpsClient_hpp
