#include "campuscompute.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

int run_worker(int fail_mode) {
    pid_t child=fork(); if(child<0){perror("fork");return 1;}
    if(child==0){if(fail_mode){fprintf(stderr,"Worker PID %ld: controlled simulated failure requested\n",(long)getpid());_exit(7);}printf("Worker PID %ld: work completed normally\n",(long)getpid());fflush(stdout);_exit(0);}
    int status;pid_t got;do{got=waitpid(child,&status,0);}while(got<0&&errno==EINTR);
    if(got<0){perror("waitpid");return 1;}
    if(WIFEXITED(status)){printf("Parent reaped PID %ld: exit status=%d%s\n",(long)child,WEXITSTATUS(status),fail_mode?" (controlled simulated failure detected)":" (normal completion)");return (fail_mode&&WEXITSTATUS(status)==7)||(!fail_mode&&WEXITSTATUS(status)==0)?0:1;}
    printf("Parent reaped PID %ld: worker terminated abnormally\n",(long)child);return fail_mode?0:1;
}

int run_limit(void) {
    struct rlimit old, requested;
    if(getrlimit(RLIMIT_NOFILE,&old)<0){perror("getrlimit");return 1;}
    requested=old; rlim_t cap=old.rlim_cur<32?old.rlim_cur:32;
    if(cap<3){fprintf(stderr,"Current descriptor soft limit is too low for this demonstration.\n");return 1;}
    requested.rlim_cur=cap;
    if(setrlimit(RLIMIT_NOFILE,&requested)<0){perror("setrlimit RLIMIT_NOFILE");return 1;}
    struct rlimit observed;if(getrlimit(RLIMIT_NOFILE,&observed)<0){perror("getrlimit after setrlimit");return 1;}
    printf("RLIMIT_NOFILE soft limit: before=%llu, applied=%llu, observed=%llu; this caps open file descriptors for this process.\n",
           (unsigned long long)old.rlim_cur,(unsigned long long)requested.rlim_cur,(unsigned long long)observed.rlim_cur);
    return observed.rlim_cur==requested.rlim_cur?0:1;
}
