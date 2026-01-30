#include <dlls/dll_lib.h>
#include <smthng_os.h>

/*
 * this file is equivalent to ntdll.dll in windows but diffrent
 * it will export some basic functions that are used by other dlls and programs
 * but i will not create dummy versions of kernel32.dll, user32.dll, etc...
 * instead i will just export the necessary functions from this dll
 * and programs will link against this dll for those functions
 */

/*
 * eq to TerminateProcess syscall
 * my os will use the kill_process name for this syscall
 * though it will also export it as TerminateProcess for compatibility and ExitProcess
 * but it will all have the same abstraction layer since i think having multiple ways to do the same thing is redundant
 */
__declspec(dllexport) u0 kill_process(u0 *handle, u8 status) {
    smthng_abi_syscall((u32)0x002c, (u32)handle, (u32)status); // 0x002c is the syscall number of TerminateProcess
}
// these all do the same thing
__declspec(dllexport) u0 (*TerminateProcess)(u0 *, u8) = kill_process;
__declspec(dllexport) u0 (*ExitProcess)(u0 *, u8)      = kill_process;