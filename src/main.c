#include "campuscompute.h"

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_int(const char *text, int minimum, int maximum, int *value) {
    char *end = NULL;
    long parsed;
    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno || end == text || *end != '\0' || parsed < minimum || parsed > maximum) return -1;
    *value = (int)parsed;
    return 0;
}

void print_usage(const char *program) {
    printf("CampusCompute: OS scheduling and resource management prototype\n"
           "Usage:\n"
           "  %s help\n"
           "  %s schedule <fcfs|rr|priority> [quantum]\n"
           "  %s compare\n"
           "  %s logger [event-count] [log-path]\n"
           "  %s ipc\n"
           "  %s banker <safe|unsafe|both>\n"
           "  %s worker <normal|fail|both>\n"
           "  %s limit\n"
           "  %s cleanup\n"
           "  %s demo\n"
           "Sample jobs are repeatable and documented in README.md.\n",
           program, program, program, program, program, program, program, program, program, program);
}

static int run_cleanup_check(void) {
    int result = 0;
    puts("Cleanup verification: joining logger threads and reaping child workers.");
    if (run_logger(4, "campuscompute.log") != 0) result = 1;
    if (run_worker(0) != 0) result = 1;
    if (run_worker(1) != 0) result = 1;
    if (run_ipc() != 0) result = 1;

    DIR *shared_memory = opendir("/dev/shm");
    if (!shared_memory) { perror("opendir /dev/shm"); return 1; }
    int leftovers = 0;
    struct dirent *entry;
    while ((entry = readdir(shared_memory)) != NULL) {
        if (strncmp(entry->d_name, "campuscompute_", 14) == 0 ||
            strncmp(entry->d_name, "sem.campuscompute_sem_", 22) == 0) {
            printf("Program-prefixed IPC name remains: %s\n", entry->d_name);
            leftovers = 1;
        }
    }
    closedir(shared_memory);
    if (!leftovers) puts("No CampusCompute IPC objects remain under /dev/shm.");
    else result = 1;
    if (result == 0) puts("Cleanup check passed: threads joined, child processes reaped, and created IPC names released.");
    return result;
}

int run_demo(void) {
    const Job jobs[] = {{1, 0, 5, 2, JOB_NEW}, {2, 1, 3, 1, JOB_NEW}, {3, 2, 8, 3, JOB_NEW}, {4, 3, 6, 1, JOB_NEW}};
    int result = 0;
    puts("=== CampusCompute integrated demonstration ===");
    puts("[1/7] Scheduling comparison");
    result |= run_schedule(SCHED_FCFS, jobs, 4, 2, 0);
    result |= run_schedule(SCHED_RR, jobs, 4, 2, 0);
    result |= run_schedule(SCHED_PRIORITY, jobs, 4, 2, 0);
    puts("[2/7] Producer-consumer logger"); result |= run_logger(8, "campuscompute.log");
    puts("[3/7] Shared-memory IPC and semaphore"); result |= run_ipc();
    puts("[4/7] Banker safe state"); result |= run_banker(0);
    puts("[5/7] Banker unsafe state"); result |= run_banker(1);
    puts("[6/7] Normal and simulated-failure workers");
    result |= run_worker(0); result |= run_worker(1);
    puts("[7/7] Resource limit"); result |= run_limit();
    printf("Integrated demonstration %s. Log: campuscompute.log\n", result ? "finished with one or more errors" : "completed");
    return result;
}

int main(int argc, char **argv) {
    const Job jobs[] = {{1, 0, 5, 2, JOB_NEW}, {2, 1, 3, 1, JOB_NEW}, {3, 2, 8, 3, JOB_NEW}, {4, 3, 6, 1, JOB_NEW}};
    if (argc < 2 || strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]); return argc < 2 ? 2 : 0;
    }
    if (strcmp(argv[1], "schedule") == 0) {
        SchedulePolicy policy;
        int quantum = 2;
        if (argc < 3 || argc > 4) goto bad_usage;
        if (strcmp(argv[2], "fcfs") == 0) policy = SCHED_FCFS;
        else if (strcmp(argv[2], "rr") == 0) policy = SCHED_RR;
        else if (strcmp(argv[2], "priority") == 0) policy = SCHED_PRIORITY;
        else goto bad_usage;
        if (argc == 4 && parse_int(argv[3], 1, INT_MAX, &quantum)) { fprintf(stderr, "Error: quantum must be a positive integer.\n"); return 2; }
        return run_schedule(policy, jobs, 4, quantum, 1);
    }
    if (strcmp(argv[1], "compare") == 0 && argc == 2) return run_comparison();
    if (strcmp(argv[1], "logger") == 0) {
        int count = 8;
        if (argc > 4) goto bad_usage;
        if (argc >= 3 && parse_int(argv[2], 1, 10000, &count)) { fprintf(stderr, "Error: event-count must be between 1 and 10000.\n"); return 2; }
        return run_logger(count, argc == 4 ? argv[3] : "campuscompute.log");
    }
    if (strcmp(argv[1], "ipc") == 0 && argc == 2) return run_ipc();
    if (strcmp(argv[1], "banker") == 0 && argc == 3) {
        if (strcmp(argv[2], "safe") == 0) return run_banker(0);
        if (strcmp(argv[2], "unsafe") == 0) return run_banker(1);
        if (strcmp(argv[2], "both") == 0) return run_banker(0) | run_banker(1);
        goto bad_usage;
    }
    if (strcmp(argv[1], "worker") == 0 && argc == 3) {
        if (strcmp(argv[2], "normal") == 0) return run_worker(0);
        if (strcmp(argv[2], "fail") == 0) return run_worker(1);
        if (strcmp(argv[2], "both") == 0) return run_worker(0) | run_worker(1);
        goto bad_usage;
    }
    if (strcmp(argv[1], "limit") == 0 && argc == 2) return run_limit();
    if (strcmp(argv[1], "cleanup") == 0 && argc == 2) return run_cleanup_check();
    if (strcmp(argv[1], "demo") == 0 && argc == 2) return run_demo();
bad_usage:
    fprintf(stderr, "Error: invalid command or arguments.\n"); print_usage(argv[0]); return 2;
}
