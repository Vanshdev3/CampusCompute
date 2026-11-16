#ifndef CAMPUSCOMPUTE_H
#define CAMPUSCOMPUTE_H

#include <stddef.h>

#define MAX_JOBS 32

typedef enum { JOB_NEW, JOB_READY, JOB_RUNNING, JOB_COMPLETED } JobStatus;

typedef struct {
    int id;
    int arrival;
    int burst;
    int priority;
    JobStatus status;
} Job;

typedef enum { SCHED_FCFS, SCHED_RR, SCHED_PRIORITY } SchedulePolicy;

int run_schedule(SchedulePolicy policy, const Job *jobs, size_t count, int quantum, int verbose);
int run_comparison(void);
int run_logger(int event_count, const char *path);
int run_ipc(void);
int run_banker(int unsafe_case);
int run_worker(int fail_mode);
int run_limit(void);
int run_demo(void);
void print_usage(const char *program);

#endif
