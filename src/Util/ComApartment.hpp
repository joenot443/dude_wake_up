// ComApartment.hpp
// RAII wrappers around CoInitializeEx / CoUninitialize. Windows-only.
//
// Apartment policy for Nottawa (see docs/WINDOWS_ARCHITECTURE.md):
//   - Main thread: UNINITIALIZED (default). We do not call CoInitializeEx on
//     the main thread. WinSparkle 0.8+ manages its own apartment on its own
//     thread; if a future dependency demands STA on the main thread we can
//     add a ComApartmentSTA guard around main() — but ONLY after confirming
//     no other subsystem needs MTA there.
//   - WASAPI capture thread: ComApartmentMTA at thread entry.
//   - Any thread that calls ShellExecuteW / IFileDialog: ComApartmentSTA
//     scoped to that call.
//
// CoInitializeEx returns:
//   S_OK          — first init on this thread; we own the ref, must CoUninit.
//   S_FALSE       — already initialized in the same mode; we own the ref, must CoUninit.
//   RPC_E_CHANGED_MODE — already initialized in a DIFFERENT mode; DO NOT CoUninit.
//                        This is a bug (someone else on this thread got there first
//                        with a conflicting mode). In debug we assert; in release we
//                        log and continue without holding a ref.
//
// The RAII guards handle all three: only balance CoUninitialize when the
// CoInitializeEx call actually returned S_OK or S_FALSE.

#ifndef ComApartment_hpp
#define ComApartment_hpp

#ifdef _WIN32

#include <windows.h>
#include <objbase.h>
#include "ofLog.h"

namespace nottawa {

class ComApartmentBase {
public:
  ComApartmentBase(const ComApartmentBase&) = delete;
  ComApartmentBase& operator=(const ComApartmentBase&) = delete;

  bool ok() const { return owns_; }
  HRESULT hresult() const { return hr_; }

protected:
  explicit ComApartmentBase(DWORD flags, const char* label)
      : hr_(CoInitializeEx(nullptr, flags)), owns_(false), label_(label) {
    if (hr_ == S_OK || hr_ == S_FALSE) {
      owns_ = true;
    } else if (hr_ == RPC_E_CHANGED_MODE) {
      ofLogError("ComApartment") << label_ << ": RPC_E_CHANGED_MODE — thread "
        "was already initialized in a different apartment mode. Not balancing "
        "CoUninitialize. This indicates a policy bug — check apartment invariants.";
    } else {
      ofLogError("ComApartment") << label_ << ": CoInitializeEx failed hr=0x"
        << std::hex << hr_;
    }
  }

  ~ComApartmentBase() {
    if (owns_) CoUninitialize();
  }

private:
  HRESULT hr_;
  bool owns_;
  const char* label_;
};

class ComApartmentMTA : public ComApartmentBase {
public:
  ComApartmentMTA() : ComApartmentBase(COINIT_MULTITHREADED, "MTA") {}
};

class ComApartmentSTA : public ComApartmentBase {
public:
  ComApartmentSTA()
      : ComApartmentBase(COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE, "STA") {}
};

// Debug-only: assert the current thread is in the expected apartment.
// Callable from any COM entry point during dev to catch policy drift early.
// Compiles to nothing in release.
#ifdef _DEBUG
inline void assertApartment(APTTYPE expected, const char* site) {
  APTTYPE t = APTTYPE_CURRENT;
  APTTYPEQUALIFIER q = APTTYPEQUALIFIER_NONE;
  HRESULT hr = CoGetApartmentType(&t, &q);
  if (FAILED(hr)) {
    ofLogError("ComApartment") << site << ": CoGetApartmentType failed hr=0x"
      << std::hex << hr;
    return;
  }
  if (t != expected) {
    ofLogError("ComApartment") << site << ": apartment mismatch — expected "
      << static_cast<int>(expected) << " got " << static_cast<int>(t)
      << " qualifier " << static_cast<int>(q);
  }
}
#else
inline void assertApartment(APTTYPE, const char*) {}
#endif

} // namespace nottawa

#endif // _WIN32
#endif // ComApartment_hpp
