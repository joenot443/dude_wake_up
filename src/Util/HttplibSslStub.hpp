//
//  HttplibSslStub.hpp
//
//  When httplib is compiled without OPENSSL support (Windows build has no
//  OpenSSL yet), httplib::SSLClient is not declared — but LibraryService.cpp
//  references it in HTTPS-fetch paths that are already guarded by an
//  #ifndef CPPHTTPLIB_OPENSSL_SUPPORT { log; return; } block. The runtime
//  never enters the SSLClient code, but the compiler still parses it.
//
//  This header supplies a header-only stub with all the methods (and
//  overloads) LibraryService touches, so the TU type-checks. Include this
//  AFTER httplib.h.
//
//  TODO(Phase 5+): remove this once we either link a Windows OpenSSL build
//  or migrate HTTPS to a Windows-native TLS backend (Schannel).
//

#ifndef HttplibSslStub_hpp
#define HttplibSslStub_hpp

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace httplib {

using Progress = std::function<bool(uint64_t current, uint64_t total)>;

class SSLClient {
public:
  explicit SSLClient(const std::string&) {}
  explicit SSLClient(const char*)        {}

  void enable_server_certificate_verification(bool) {}
  void set_connection_timeout(int, int)             {}
  void set_read_timeout(int, int)                   {}
  void set_write_timeout(int, int)                  {}
  void set_follow_location(bool)                    {}
  void set_default_headers(const Headers&)          {}

  // Every Result in this stub returns an empty (falsy) Result so an
  // `if (res && res->status == 200)` on the calling side folds to false.
  // The default-constructed Response* is nullptr; the empty Error keeps
  // Result usable but "no response received."
  static Result emptyResult() { return Result{nullptr, Error::Unknown}; }

  Result Get(const char*)                                { return emptyResult(); }
  Result Get(const char*, const Headers&)                { return emptyResult(); }
  Result Get(const char*, const Headers&, Progress)      { return emptyResult(); }
  Result Get(const std::string&)                         { return emptyResult(); }
  Result Get(const std::string&, const Headers&)         { return emptyResult(); }
  Result Get(const std::string&, const Headers&, Progress) { return emptyResult(); }

  Result Post(const char*)                               { return emptyResult(); }
  Result Post(const char*, const std::string&, const char*) { return emptyResult(); }
  Result Post(const char*, const Headers&, const std::string&, const char*) { return emptyResult(); }
  Result Post(const char*, const Headers&, const MultipartFormDataItems&) { return emptyResult(); }
  Result Post(const std::string&, const Headers&, const MultipartFormDataItems&) { return emptyResult(); }

  Result Head(const char*)                { return emptyResult(); }
  Result Head(const std::string&)         { return emptyResult(); }
};

} // namespace httplib

#endif // !CPPHTTPLIB_OPENSSL_SUPPORT
#endif // HttplibSslStub_hpp
