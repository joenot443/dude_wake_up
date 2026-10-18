#include "CrashHandler.h"

#include <windows.h>
#include <dbghelp.h>

#include <cstdio>
#include <cstring>

#pragma comment(lib, "dbghelp.lib")

namespace CrashHandler {

namespace {

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

    alignas(SYMBOL_INFO) char buffer[sizeof(SYMBOL_INFO) + 256] = {};
    auto* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
    for (int i = 0; i < 48; ++i) {
        if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, thread, &frame, &context, nullptr,
                         SymFunctionTableAccess64, SymGetModuleBase64, nullptr) || !frame.AddrPC.Offset)
            break;
        const DWORD64 address = frame.AddrPC.Offset;
        char module[MAX_PATH] = "?";
        if (const DWORD64 base = SymGetModuleBase64(process, address)) {
            GetModuleFileNameA(reinterpret_cast<HMODULE>(base), module, MAX_PATH);
            const char* slash = std::strrchr(module, '\\');
            if (slash) std::memmove(module, slash + 1, std::strlen(slash));
        }
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = 255;
        DWORD64 displacement = 0;
        IMAGEHLP_LINE64 line = {sizeof(IMAGEHLP_LINE64)};
        DWORD lineDisplacement = 0;
        const bool named = SymFromAddr(process, address, &displacement, symbol);
        if (SymGetLineFromAddr64(process, address, &lineDisplacement, &line))
            std::fprintf(stderr, "  #%02d %s!%s  %s:%lu\n", i, module, named ? symbol->Name : "?", line.FileName, line.LineNumber);
        else
            std::fprintf(stderr, "  #%02d %s!%s+0x%llx\n", i, module, named ? symbol->Name : "?",
                         named ? displacement : address);
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

} // anon

void install() {
    SetUnhandledExceptionFilter(&onCrash);
    AddVectoredExceptionHandler(1, &onFailFast);
}

} // namespace CrashHandler
