#include "campuscompute.h"

#include <stdio.h>
#include <string.h>

typedef struct { int completion, turnaround, waiting; } Metrics;

static const char *policy_name(SchedulePolicy p) {
    return p == SCHED_FCFS ? "FCFS" : p == SCHED_RR ? "Round Robin" : "Non-preemptive Priority";
}

static void print_metrics(const Job *jobs, size_t count, const Metrics *m) {
    double total_wait = 0, total_turn = 0;
    puts("Job  Arrival  Burst  Priority  Completion  Turnaround  Waiting");
    for (size_t i = 0; i < count; ++i) {
        printf("J%-3d %-8d %-6d %-9d %-11d %-11d %d\n", jobs[i].id, jobs[i].arrival, jobs[i].burst,
               jobs[i].priority, m[i].completion, m[i].turnaround, m[i].waiting);
        total_wait += m[i].waiting; total_turn += m[i].turnaround;
    }
    printf("Average turnaround: %.2f | Average waiting: %.2f\n", total_turn / count, total_wait / count);
}

int run_schedule(SchedulePolicy policy, const Job *jobs, size_t count, int quantum, int verbose) {
    Metrics m[MAX_JOBS] = {{0}};
    int order[4096], starts[4096], ends[4096], slices = 0, time = 0;
    int done[MAX_JOBS] = {0};
    if (!jobs || count == 0 || count > MAX_JOBS || (policy == SCHED_RR && quantum <= 0)) return 1;
    if (verbose) printf("Policy: %s%s\n", policy_name(policy), policy == SCHED_RR ? " (quantum=2 by default)" : "");
    if (policy == SCHED_FCFS || policy == SCHED_PRIORITY) {
        size_t completed = 0;
        while (completed < count) {
            int pick = -1;
            for (size_t i = 0; i < count; ++i) {
                if (done[i] || jobs[i].arrival > time) continue;
                if (pick < 0 || (policy == SCHED_FCFS && (jobs[i].arrival < jobs[pick].arrival || (jobs[i].arrival == jobs[pick].arrival && jobs[i].id < jobs[pick].id))) ||
                    (policy == SCHED_PRIORITY && (jobs[i].priority < jobs[pick].priority || (jobs[i].priority == jobs[pick].priority && (jobs[i].arrival < jobs[pick].arrival || (jobs[i].arrival == jobs[pick].arrival && jobs[i].id < jobs[pick].id)))))) pick = (int)i;
            }
            if (pick < 0) {
                int next = -1; for (size_t i = 0; i < count; ++i) if (!done[i] && (next < 0 || jobs[i].arrival < jobs[next].arrival)) next = (int)i;
                time = jobs[next].arrival; continue;
            }
            starts[slices] = time; time += jobs[pick].burst; ends[slices] = time; order[slices++] = pick;
            m[pick].completion = time; m[pick].turnaround = time - jobs[pick].arrival;
            m[pick].waiting = m[pick].turnaround - jobs[pick].burst; done[pick] = 1; ++completed;
        }
    } else {
        int remaining[MAX_JOBS]; int completed = 0;
        for (size_t i = 0; i < count; ++i) remaining[i] = jobs[i].burst;
        while (completed < (int)count) {
            int progressed = 0;
            for (size_t scan = 0; scan < count; ++scan) {
                int pick = -1;
                for (size_t i = 0; i < count; ++i) if (remaining[i] > 0 && jobs[i].arrival <= time && (pick < 0 || jobs[i].arrival < jobs[pick].arrival || (jobs[i].arrival == jobs[pick].arrival && jobs[i].id < jobs[pick].id))) pick = (int)i;
                if (pick < 0) break;
                int run = remaining[pick] < quantum ? remaining[pick] : quantum;
                starts[slices] = time; time += run; ends[slices] = time; order[slices++] = pick; remaining[pick] -= run; progressed = 1;
                if (!remaining[pick]) { m[pick].completion = time; m[pick].turnaround = time - jobs[pick].arrival; m[pick].waiting = m[pick].turnaround - jobs[pick].burst; ++completed; }
                if (slices >= 4090) return 1;
            }
            if (!progressed) ++time;
        }
    }
    if (verbose) {
        printf("Tie-break: earlier arrival, then lower job ID; Priority uses lower numeric value first.\n");
        if (policy == SCHED_RR) printf("Time quantum: %d\n", quantum);
        printf("Timeline: ");
        for (int i = 0; i < slices; ++i) printf("[%d-%d J%d]%s", starts[i], ends[i], jobs[order[i]].id, i + 1 == slices ? "\n" : " ");
        print_metrics(jobs, count, m);
    } else {
        double total_turn = 0, total_wait = 0;
        for (size_t i = 0; i < count; ++i) { total_turn += m[i].turnaround; total_wait += m[i].waiting; }
        printf("%s: %d execution segment(s), avg turnaround %.2f, avg waiting %.2f\n", policy_name(policy), slices,
               total_turn / count, total_wait / count);
    }
    return 0;
}

int run_comparison(void) {
    const Job jobs[] = {{1, 0, 5, 2}, {2, 1, 3, 1}, {3, 2, 8, 3}, {4, 3, 6, 1}};
    puts("Same job set for all policies: J1(0,5,p2), J2(1,3,p1), J3(2,8,p3), J4(3,6,p1)");
    run_schedule(SCHED_FCFS, jobs, 4, 2, 0); run_schedule(SCHED_RR, jobs, 4, 2, 0); run_schedule(SCHED_PRIORITY, jobs, 4, 2, 0);
    return 0;
}
