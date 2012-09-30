#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <unistd.h>
#include <limits.h>
#include <sys/syscall.h>

struct SyscallSpec {
    int number, limit;
};
#ifdef __x86_64__
// FIXME: the sys calls in 64 bit OS is not well tested
const static SyscallSpec g_spec[] = {
    {SYS_write, INT_MAX},
    {SYS_read, INT_MAX},
    {SYS_brk, INT_MAX},
    {SYS_access, INT_MAX},
    {SYS_mmap, INT_MAX},
    {SYS_munmap, INT_MAX},
    {SYS_execve, 1},
    {SYS_fstat, INT_MAX},
    {SYS_arch_prctl, 1},
    {SYS_uname, 1},
    {SYS_gettimeofday, INT_MAX},
    {SYS_getrusage, INT_MAX},
    {SYS_time, INT_MAX},
    {-1, 0}
};
#else
const static SyscallSpec g_spec[] = {
    {SYS_write, INT_MAX},
    {SYS_read, INT_MAX},
    {SYS_brk, INT_MAX},
    {SYS_access, INT_MAX},
    {SYS_mmap2, INT_MAX},
    {SYS_munmap, INT_MAX},
    {SYS_set_thread_area, 1},
    {SYS_fstat64, INT_MAX},
    {SYS_uname, 1},
    {SYS_execve, 1},
    {SYS_getpid, INT_MAX},
    {SYS_gettimeofday, INT_MAX},
    {SYS_getrusage, INT_MAX},
    {SYS_time, INT_MAX},
    {-1, 0}
};
#endif
#endif
