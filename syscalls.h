#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <unistd.h>
#include <limits.h>
#include <sys/syscall.h>

struct SyscallSpec {
    int number, limit;
};
#ifdef __x86_64__
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
    {-1, 0}
};
#endif
#endif
