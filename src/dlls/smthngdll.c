#include <dlls/dll_lib.h>
#include <smthng_os.h>

// eq to TerminateProcess syscall
__declspec(dllexport) u0 kill_process(u0 *handle, u8 status) {
    smthng_abi_syscall((u32)0x002c, (u32)handle, (u32)status); // 0x002c is the syscall number of TerminateProcess
}