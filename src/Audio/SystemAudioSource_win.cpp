// SystemAudioSource_win.cpp
// Windows WASAPI loopback capture. Captures the default render device's
// output stream (the same thing you'd hear from the speakers) and feeds
// AudioSource::audioIn() so the visual reactor treats it identically to
// microphone/file input.
//
// Threading:
//   - COM is NEVER initialized on the main thread (see docs/WINDOWS_ARCHITECTURE.md).
//   - The capture thread owns the entire COM lifetime via ComApartmentMTA.
//   - setup() spawns the thread; disable() signals shutdown, joins, and
//     lets the thread destructor tear the ComPtrs down and CoUninitialize.
//   - AudioSource::audioIn() (which fires recordingCallback then runs
//     Gist) is invoked on the capture thread. That matches macOS's
//     Core Audio IOProc behavior, so the existing AudioSource contract
//     (recording callback is invoked on audio thread) holds unchanged.
//
// Format:
//   - GetMixFormat returns the mixer's shared-mode format. On Vista+ that
//     is always float32 — we verify with an explicit format-tag check and
//     bail loudly if a driver ever violates that assumption.
//   - AUDCLNT_STREAMFLAGS_LOOPBACK opens the render endpoint for capture.
//   - AUDCLNT_STREAMFLAGS_EVENTCALLBACK gives us an event to wait on
//     instead of polling. Combined with a modest buffer (100 ms), latency
//     is acceptable for a visual reactor without spinning a hot loop.
//
// Per-process loopback (Win 10 2004+ via ActivateAudioInterfaceAsync +
// AUDIOCLIENT_ACTIVATION_PARAMS) is out of scope for v1 — the current
// mac codebase only exercises the global tap. Widening to per-process
// is a future task once the global path is proven.

#ifdef _WIN32

#include "SystemAudioSource.hpp"
#include "ComApartment.hpp"
#include "ofLog.h"
#include "ofSoundBuffer.h"

#include <avrt.h>              // AvSetMmThreadCharacteristicsW
#include <functiondiscoverykeys_devpkey.h>
#include <ksmedia.h>           // KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
#include <chrono>
#include <cstring>
#include <cstdio>

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Avrt.lib")
#pragma comment(lib, "Propsys.lib")   // PropVariantClear on some SDK layouts
#pragma comment(lib, "Uuid.lib")      // fallback GUID resolution

using Microsoft::WRL::ComPtr;

namespace {

// GUIDs. Declaring inline so we don't depend on the linker finding
// them in a specific SDK version's lib.
const CLSID CLSID_MMDeviceEnumerator_local = __uuidof(MMDeviceEnumerator);
const IID   IID_IMMDeviceEnumerator_local  = __uuidof(IMMDeviceEnumerator);
const IID   IID_IAudioClient_local         = __uuidof(IAudioClient);
const IID   IID_IAudioCaptureClient_local  = __uuidof(IAudioCaptureClient);

constexpr REFERENCE_TIME kHundredNanoPerSecond = 10000000LL;
constexpr REFERENCE_TIME kBufferDurationHns    = 1000000LL;   // 100 ms buffer

// Log Win32 HRESULTs decoded to hex.
void logHr(const char* site, HRESULT hr) {
  ofLogError("SystemAudioSource_win") << site << " failed hr=0x"
    << std::hex << static_cast<unsigned int>(hr) << std::dec;
}

// Best-effort: read the device's friendly name for the log line.
std::string deviceFriendlyName(IMMDevice* device) {
  if (!device) return "(null)";
  ComPtr<IPropertyStore> props;
  if (FAILED(device->OpenPropertyStore(STGM_READ, &props))) return "(unavailable)";
  PROPVARIANT pv;
  PropVariantInit(&pv);
  std::string out = "(unavailable)";
  if (SUCCEEDED(props->GetValue(PKEY_Device_FriendlyName, &pv)) && pv.vt == VT_LPWSTR && pv.pwszVal) {
    int need = WideCharToMultiByte(CP_UTF8, 0, pv.pwszVal, -1, nullptr, 0, nullptr, nullptr);
    if (need > 0) {
      out.resize(static_cast<size_t>(need - 1));
      WideCharToMultiByte(CP_UTF8, 0, pv.pwszVal, -1, out.data(), need, nullptr, nullptr);
    }
  }
  PropVariantClear(&pv);
  return out;
}

// Verify a WAVEFORMATEX* actually describes 32-bit float PCM. Shared-mode
// mixer output is always float32 on Vista+, so a violation is a driver
// bug — we log and bail rather than convert.
bool isFloat32(const WAVEFORMATEX* wfx) {
  if (!wfx) return false;
  if (wfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) return true;
  if (wfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
    const auto* ext = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(wfx);
    return IsEqualGUID(ext->SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
  }
  return false;
}

} // namespace

SystemAudioSource::SystemAudioSource()  {}
SystemAudioSource::~SystemAudioSource() { stopCaptureThread_(); }

void SystemAudioSource::setup() {
  if (running_.load()) {
    ofLogWarning("SystemAudioSource_win") << "setup() called while already running — ignoring";
    return;
  }
  running_ = true;
  captureThread_ = std::thread([this]() { captureLoop_(); });
  active = true;
}

void SystemAudioSource::disable() {
  stopCaptureThread_();
  active = false;
}

void SystemAudioSource::stopCaptureThread_() {
  // Signal shutdown even if running_ was already false (thread may have
  // exited on its own from inside captureLoop_'s cleanup path — in that
  // case we still need to join to avoid std::terminate on destruction).
  //
  // Deliberately do NOT SetEvent(captureEvent_) from here: the event is
  // created and closed inside captureLoop_, so touching it from the main
  // thread would race with the thread's cleanup. The 200 ms
  // WaitForSingleObject timeout in the loop bounds our shutdown latency
  // instead — good enough for a user-triggered stop.
  running_ = false;
  if (captureThread_.joinable()) captureThread_.join();
}

void SystemAudioSource::captureLoop_() {
  // Own COM for the thread's lifetime.
  nottawa::ComApartmentMTA com;
  if (!com.ok()) {
    ofLogError("SystemAudioSource_win") << "COM init failed — abandoning capture";
    running_ = false;
    return;
  }
  nottawa::assertApartment(APTTYPE_MTA, "SystemAudioSource_win::captureLoop_");

  // Boost thread priority for audio. Not fatal if this fails.
  DWORD mmcssTaskIndex = 0;
  HANDLE mmcss = AvSetMmThreadCharacteristicsW(L"Pro Audio", &mmcssTaskIndex);
  if (!mmcss) {
    ofLogWarning("SystemAudioSource_win") << "AvSetMmThreadCharacteristicsW('Pro Audio') failed — capture will run at normal priority";
  }

  HRESULT hr = S_OK;

  // Enumerate default render device (we're going to loopback-capture it).
  ComPtr<IMMDeviceEnumerator> enumerator;
  hr = CoCreateInstance(CLSID_MMDeviceEnumerator_local, nullptr, CLSCTX_ALL,
                        IID_IMMDeviceEnumerator_local, &enumerator);
  if (FAILED(hr)) { logHr("CoCreateInstance(MMDeviceEnumerator)", hr); goto cleanup; }

  hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device_);
  if (FAILED(hr)) { logHr("GetDefaultAudioEndpoint", hr); goto cleanup; }
  ofLogNotice("SystemAudioSource_win") << "device: " << deviceFriendlyName(device_.Get());

  // Activate an IAudioClient on it.
  hr = device_->Activate(IID_IAudioClient_local, CLSCTX_ALL, nullptr, &audioClient_);
  if (FAILED(hr)) { logHr("IMMDevice::Activate(IAudioClient)", hr); goto cleanup; }

  // Query the mixer's shared-mode format (always float32 on Vista+).
  {
    WAVEFORMATEX* mixWfx = nullptr;
    hr = audioClient_->GetMixFormat(&mixWfx);
    if (FAILED(hr) || !mixWfx) { logHr("GetMixFormat", hr); goto cleanup; }

    if (!isFloat32(mixWfx)) {
      ofLogError("SystemAudioSource_win") << "mixer format is not float32 (wFormatTag=0x"
        << std::hex << mixWfx->wFormatTag << std::dec << ") — WASAPI capture requires float32";
      CoTaskMemFree(mixWfx);
      goto cleanup;
    }

    // Copy into our WAVEFORMATEXTENSIBLE-shaped buffer. If the driver
    // returned bare WAVEFORMATEX (cbSize=0), we memcpy only the base
    // struct's bytes to avoid reading past what was allocated.
    const size_t copyBytes = (mixWfx->cbSize >= sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))
                              ? sizeof(WAVEFORMATEXTENSIBLE)
                              : sizeof(WAVEFORMATEX);
    std::memset(&mixFormat_, 0, sizeof(mixFormat_));
    std::memcpy(&mixFormat_, mixWfx, copyBytes);

    sampleRate_ = static_cast<uint32_t>(mixWfx->nSamplesPerSec);
    channels_   = static_cast<uint16_t>(mixWfx->nChannels);
    CoTaskMemFree(mixWfx);
  }
  ofLogNotice("SystemAudioSource_win") << "mix format: " << sampleRate_
    << " Hz, " << channels_ << "ch float32";

  // Preflight Gist. Doing this BEFORE the first packet arrives avoids a
  // realloc race if the audio thread hits processAudioFrame while the
  // main thread is still adjusting Gist state.
  gist.setSamplingFrequency(sampleRate_);
  gist.setAudioFrameSize(512);

  // Prepare a manual-reset event for AUDCLNT_STREAMFLAGS_EVENTCALLBACK.
  captureEvent_ = CreateEventW(nullptr, FALSE, FALSE, nullptr);
  if (!captureEvent_) {
    ofLogError("SystemAudioSource_win") << "CreateEventW failed lastError="
      << GetLastError();
    goto cleanup;
  }

  hr = audioClient_->Initialize(
    AUDCLNT_SHAREMODE_SHARED,
    AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
    kBufferDurationHns, 0,
    reinterpret_cast<WAVEFORMATEX*>(&mixFormat_), nullptr);
  if (FAILED(hr)) { logHr("IAudioClient::Initialize", hr); goto cleanup; }

  hr = audioClient_->SetEventHandle(captureEvent_);
  if (FAILED(hr)) { logHr("IAudioClient::SetEventHandle", hr); goto cleanup; }

  hr = audioClient_->GetService(IID_IAudioCaptureClient_local, &captureClient_);
  if (FAILED(hr)) { logHr("IAudioClient::GetService(IAudioCaptureClient)", hr); goto cleanup; }

  hr = audioClient_->Start();
  if (FAILED(hr)) { logHr("IAudioClient::Start", hr); goto cleanup; }

  // ---------- capture loop ----------
  while (running_.load()) {
    DWORD waited = WaitForSingleObject(captureEvent_, 200);
    if (!running_.load()) break;
    if (waited == WAIT_TIMEOUT) continue;  // no data yet; re-check running_
    if (waited != WAIT_OBJECT_0) {
      ofLogError("SystemAudioSource_win") << "WaitForSingleObject unexpected=" << waited;
      break;
    }

    // Drain all packets currently available. A single event fire can
    // correspond to multiple packets under heavy CPU load.
    UINT32 packetSize = 0;
    hr = captureClient_->GetNextPacketSize(&packetSize);
    if (FAILED(hr)) { logHr("GetNextPacketSize", hr); break; }

    while (packetSize > 0) {
      BYTE*  data     = nullptr;
      UINT32 numFrames = 0;
      DWORD  flags    = 0;

      hr = captureClient_->GetBuffer(&data, &numFrames, &flags, nullptr, nullptr);
      if (FAILED(hr)) { logHr("GetBuffer", hr); goto cleanup; }

      // Build an ofSoundBuffer for audioIn(). If the packet is flagged
      // AUDCLNT_BUFFERFLAGS_SILENT, WASAPI didn't necessarily fill data;
      // we synthesize zeros of the right shape to keep timing stable.
      ofSoundBuffer buf;
      buf.setNumChannels(channels_);
      buf.setSampleRate(sampleRate_);
      buf.resize(static_cast<size_t>(numFrames) * channels_);

      if ((flags & AUDCLNT_BUFFERFLAGS_SILENT) || !data) {
        std::memset(buf.getBuffer().data(), 0,
                    buf.getBuffer().size() * sizeof(float));
      } else {
        std::memcpy(buf.getBuffer().data(), data,
                    static_cast<size_t>(numFrames) * channels_ * sizeof(float));
      }

      captureClient_->ReleaseBuffer(numFrames);

      // audioIn handles recordingCallback under recordingMutex, then runs
      // Gist + BTrack. This is the same entry AudioSourceService uses on
      // macOS's IOProc, so the downstream analysis pipeline is unchanged.
      audioIn(buf);

      hr = captureClient_->GetNextPacketSize(&packetSize);
      if (FAILED(hr)) { logHr("GetNextPacketSize (drain)", hr); goto cleanup; }
    }
  }

  if (audioClient_) audioClient_->Stop();

cleanup:
  if (mmcss) AvRevertMmThreadCharacteristics(mmcss);
  captureClient_.Reset();
  audioClient_.Reset();
  device_.Reset();
  if (captureEvent_) { CloseHandle(captureEvent_); captureEvent_ = nullptr; }
  running_ = false;
  // ComApartmentMTA dtor runs here → CoUninitialize.
}

void SystemAudioSource::setTargetProcess(pid_t pid) {
  // Per-process loopback (Win 10 2004+, ActivateAudioInterfaceAsync +
  // AUDIOCLIENT_ACTIVATION_PARAMS) is out of scope for v1. Store the
  // requested pid so serialize() round-trips cleanly and so callers can
  // introspect what they asked for, but capture stays global.
  targetPID = pid;
  if (pid != 0) {
    ofLogWarning("SystemAudioSource_win") << "per-process loopback (pid="
      << pid << ") not implemented — capture stays global";
  }
}

void SystemAudioSource::setGlobalTap(bool global) { isGlobalTap = global; }

json SystemAudioSource::serialize() {
  json j;
  j["type"]        = static_cast<int>(AudioSourceType_System);
  j["id"]          = id;
  j["name"]        = name;
  j["targetPID"]   = targetPID;
  j["isGlobalTap"] = isGlobalTap;
  return j;
}

void SystemAudioSource::load(json j) {
  if (j.contains("id"))          id          = j["id"];
  if (j.contains("name"))        name        = j["name"];
  if (j.contains("targetPID"))   targetPID   = j["targetPID"];
  if (j.contains("isGlobalTap")) isGlobalTap = j["isGlobalTap"];
}

#endif // _WIN32
