#include "campuscompute.h"

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct { int ready; char payload[128]; } SharedMessage;

int run_ipc(void) {
    char shm_name[64], sem_name[64]; SharedMessage *shared = MAP_FAILED; sem_t *sem = SEM_FAILED; int fd = -1, status = 0, result = 0;
    snprintf(shm_name, sizeof(shm_name), "/campuscompute_%ld", (long)getpid());
    snprintf(sem_name, sizeof(sem_name), "/campuscompute_sem_%ld", (long)getpid());
    fd = shm_open(shm_name, O_CREAT | O_EXCL | O_RDWR, 0600); if (fd < 0) { perror("shm_open"); return 1; }
    if (ftruncate(fd, sizeof(SharedMessage)) < 0) { perror("ftruncate"); result=1; goto cleanup; }
    shared = mmap(NULL, sizeof(SharedMessage), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); if (shared == MAP_FAILED) { perror("mmap"); result=1; goto cleanup; }
    memset(shared, 0, sizeof(*shared)); sem = sem_open(sem_name, O_CREAT | O_EXCL, 0600, 0); if (sem == SEM_FAILED) { perror("sem_open"); result=1; goto cleanup; }
    pid_t child = fork(); if (child < 0) { perror("fork"); result=1; goto cleanup; }
    if (child == 0) {
        snprintf(shared->payload, sizeof(shared->payload), "message produced by child PID %ld", (long)getpid()); shared->ready = 1;
        if (sem_post(sem) < 0) _exit(2); _exit(0);
    }
    while (sem_wait(sem) < 0) { if (errno == EINTR) continue; perror("sem_wait"); result=1; break; }
    if (waitpid(child, &status, 0) < 0) { perror("waitpid"); result=1; }
    if (!result && WIFEXITED(status) && WEXITSTATUS(status) == 0 && shared->ready) printf("Parent PID %ld read shared-memory payload: %s\nSemaphore signaled; child exited normally.\n", (long)getpid(), shared->payload);
    else { fprintf(stderr, "IPC child or synchronization failed.\n"); result=1; }
cleanup:
    if (shared != MAP_FAILED) munmap(shared, sizeof(SharedMessage)); if (fd >= 0) close(fd);
    if (sem != SEM_FAILED) sem_close(sem); if (sem_unlink(sem_name) < 0 && errno != ENOENT) { perror("sem_unlink"); result=1; }
    if (shm_unlink(shm_name) < 0 && errno != ENOENT) { perror("shm_unlink"); result=1; }
    printf("Program-created shared memory and semaphore cleaned up.\n"); return result;
}
