#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
 
#include <wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/epoll.h>
 
#define DR_OFFSET(num) 	((void *) (& ((struct user *) 0)->u_debugreg[num]))
 
int set_hwbp(pid_t pid, void *addr)
{
    unsigned long dr_7 = 0;
 
    if (ptrace(PTRACE_POKEUSER, pid, DR_OFFSET(0), addr) != 0) {
        perror("tracer, faile to set DR_0\n");
        return 1;
    }
 
    dr_7 = dr_7 | 0x01;  // set dr_0 local
    dr_7 = dr_7 | 0x02;  // set dr_0 global
    // dr_7 = dr_7 & ((~0x0f) << 16);  // type exec, and len 0
    if (ptrace(PTRACE_POKEUSER, pid, DR_OFFSET(7), (void *) dr_7) != 0) {
        perror("tracer, faile to set DR_7\n");
        return 1;
    }
 
    return 0;
}
 