#include "CrashHandler.h"

#include <windows.h>
#include <dbghelp.h>
#include <tlhelp32.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>
#include <vector>

#pragma comment(lib, "dbghelp.lib")

namespace CrashHandler {

namespace {

constexpr int kMaxFrames = 48;

// Writes one symbolised frame. DbgHelp must already be initialised.
void printFrame(HANDLE process, int index, DWORD64 address) {
    char module[MAX_PATH] = "?";
    if (const DWORD64 base = SymGetModuleBase64(process, address)) {
        GetModuleFileNameA(reinterpret_cast<HMODULE>(base), module, MAX_PATH);
        const char* slash = std::strrchr(module, '\\');
        if (slash) std::memmove(module, slash + 1, std::strlen(slash));
    }
    alignas(SYMBOL_INFO) char buffer[sizeof(SYMBOL_INFO) + 256] = {};
    auto* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = 255;
    DWORD64 displacement = 0;
    IMAGEHLP_LINE64 line = {sizeof(IMAGEHLP_LINE64)};
    DWORD lineDisplacement = 0;
    const bool named = SymFromAddr(process, address, &displacement, symbol);
    if (SymGetLineFromAddr64(process, address, &lineDisplacement, &line))
        std::fprintf(stderr, "  #%02d %s!%s  %s:%lu\n", index, module, named ? symbol->Name : "?", line.FileName, line.LineNumber);
    else
        std::fprintf(stderr, "  #%02d %s!%s+0x%llx\n", index, module, named ? symbol->Name : "?",
                     named ? displacement : address);
}

LONG WINAPI onCrash(EXCEPTION_POINTERS* info) {
    std::fprintf(stderr, "\n=== CRASH: exception 0x%08lX at %p ===\n",
                 info->ExceptionRecord->ExceptionCode, info->ExceptionRecord->ExceptionAddress);
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();
    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
    SymInitialize(process, nullptr, TRUE);

    CONTEXT context = *info->ContextRecord;
    STACKFRAME64 frame = {};
    frame.AddrPC.Offset = context.Rip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context.Rbp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.Rsp;
    frame.AddrStack.Mode = AddrModeFlat;

    for (int i = 0; i < kMaxFrames; ++i) {
        if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, thread, &frame, &context, nullptr,
                         SymFunctionTableAccess64, SymGetModuleBase64, nullptr) || !frame.AddrPC.Offset)
            break;
        printFrame(process, i, frame.AddrPC.Offset);
    }
    std::fflush(stderr);
    SymCleanup(process);
    return EXCEPTION_CONTINUE_SEARCH;
}

// Heap corruption and stack-buffer overruns end the process as fail-fasts
// that never reach the unhandled-exception filter; a vectored handler sees
// them first. It only reports, leaving the failure to proceed.
LONG WINAPI onFailFast(EXCEPTION_POINTERS* info) {
    const DWORD code = info->ExceptionRecord->ExceptionCode;
    if (code == STATUS_HEAP_CORRUPTION || code == STATUS_STACK_BUFFER_OVERRUN) onCrash(info);
    return EXCEPTION_CONTINUE_SEARCH;
}

// A thread's return addresses, unwound from a context. Allocates nothing, so
// it's safe while the thread is suspended (it may hold the heap lock).
int unwind(CONTEXT context, DWORD64* out) {
    int count = 0;
    while (count < kMaxFrames && context.Rip) {
        out[count++] = context.Rip;
        DWORD64 imageBase = 0;
        PRUNTIME_FUNCTION function = RtlLookupFunctionEntry(context.Rip, &imageBase, nullptr);
        if (!function) {  // a leaf function: the return address is on top of the stack
            context.Rip = *reinterpret_cast<DWORD64*>(context.Rsp);
            context.Rsp += 8;
            continue;
        }
        PVOID handlerData = nullptr;
        DWORD64 establisherFrame = 0;
        RtlVirtualUnwind(UNW_FLAG_NHANDLER, imageBase, context.Rip, function, &context, &handlerData,
                         &establisherFrame, nullptr);
    }
    return count;
}

std::atomic<long long> lastBeat{0};
DWORD uiThreadId = 0;

long long nowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch()).count();
}

// Every thread's stack (the UI thread's first), written to stderr.
void dumpAllThreads(long long stalledMs) {
    struct Stack { DWORD id; int count; DWORD64 frames[kMaxFrames]; };
    std::vector<Stack> stacks;
    stacks.reserve(64);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return;
    THREADENTRY32 entry = {sizeof(entry)};
    const DWORD self = GetCurrentThreadId();
    std::vector<DWORD> ids;
    for (BOOL ok = Thread32First(snapshot, &entry); ok; ok = Thread32Next(snapshot, &entry))
        if (entry.th32OwnerProcessID == GetCurrentProcessId() && entry.th32ThreadID != self) ids.push_back(entry.th32ThreadID);
    CloseHandle(snapshot);
    for (DWORD id : ids) {
        HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, id);
        if (!thread) continue;
        if (stacks.size() == stacks.capacity()) { CloseHandle(thread); break; }  // no allocation while suspended
        stacks.push_back({id, 0, {}});
        if (SuspendThread(thread) != DWORD(-1)) {
            CONTEXT context = {};
            context.ContextFlags = CONTEXT_FULL;
            if (GetThreadContext(thread, &context)) stacks.back().count = unwind(context, stacks.back().frames);
            ResumeThread(thread);
        }
        CloseHandle(thread);
    }

    HANDLE process = GetCurrentProcess();
    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
    SymInitialize(process, nullptr, TRUE);
    std::fprintf(stderr, "\n=== HANG: the UI thread has been unresponsive for %lld ms ===\n", stalledMs);
    for (int pass = 0; pass < 2; ++pass) {
        for (const Stack& stack : stacks) {
            if ((stack.id == uiThreadId) != (pass == 0)) continue;
            std::fprintf(stderr, "-- thread %lu%s\n", stack.id, stack.id == uiThreadId ? " (UI)" : "");
            for (int i = 0; i < stack.count; ++i) printFrame(process, i, stack.frames[i]);
        }
    }
    std::fflush(stderr);
    SymCleanup(process);
}

} // anon

void install() {
    SetUnhandledExceptionFilter(&onCrash);
    AddVectoredExceptionHandler(1, &onFailFast);
}

void heartbeat() {
    lastBeat.store(nowMs());
}

void startHangWatchdog(int stallMs) {
    uiThreadId = GetCurrentThreadId();
    heartbeat();
    std::thread([stallMs] {
        bool reported = false;
        for (;;) {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            const long long stalled = nowMs() - lastBeat.load();
            if (stalled < stallMs) { reported = false; continue; }
            if (!reported) { dumpAllThreads(stalled); reported = true; }  // once per stall
        }
    }).detach();
}

} // namespace CrashHandler
