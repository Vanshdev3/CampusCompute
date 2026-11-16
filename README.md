# CampusCompute

CampusCompute is a Linux/Ubuntu command-line mini-project for operating systems. It implements CPU scheduling, a POSIX-threaded bounded logger, shared-memory IPC coordinated by a named semaphore, Banker's safety analysis, worker process status collection, and a resource-limit demonstration.

## Requirements

- Linux/Ubuntu or WSL Ubuntu
- GCC and GNU Make
- POSIX threads, shared memory, named semaphores, `fork`, `waitpid`, and `setrlimit`

## Build and run

```sh
make clean && make
./campuscompute help
./campuscompute schedule fcfs
./campuscompute schedule rr 2
./campuscompute schedule priority
./campuscompute compare
./campuscompute logger 8 campuscompute.log
./campuscompute ipc
./campuscompute banker both
./campuscompute worker both
./campuscompute limit
./campuscompute demo
```

Use lower numeric priority as higher priority. Scheduling ties use earlier arrival time, then lower job ID; FCFS uses arrival then ID. RR defaults to a quantum of 2. The same deterministic sample set is used throughout: J1=(arrival 0, burst 5, priority 2), J2=(1,3,1), J3=(2,8,3), J4=(3,6,1). Turnaround = completion - arrival; waiting = turnaround - burst.

`banker safe` and `banker unsafe` use the classic five-process, three-resource allocation/maxima sample; the unsafe run sets Available to (0,0,0). `worker fail` exits with status 7 by design and is explicitly labeled as a simulated failure. `limit` reduces the current process soft `RLIMIT_NOFILE` to at most 32; run it as a standalone command because the limit applies to that process and its descendants.

The logger replaces the selected log file. IPC names include the parent PID and the program unlinks only the names it created. `make clean` removes object files, the executable, and the default log file.

## Evidence PDF and submission

Capture genuine screenshots in Ubuntu/WSL after building and running each command. Before capture, set the prompt with the submitting student's full actual SRN, for example:

```sh
export SRN='YOUR_ACTUAL_FULL_SRN'
export PS1='[$SRN] \w\$ '
```

Do not leave the placeholder in the final evidence. Screenshots must show the command and useful output; number them S01 through S24 and add one short explanatory sentence beneath each. Combine them in one PDF named only with the submitting student's SRN (`ACTUALSRN.pdf`). Do not submit a separate report. If the course portal requires a ZIP, include the source/build files and verify the archive with `unzip -t`. Follow any portal-specific directions. Both team members should understand and be ready to explain the implementation for the viva.

## Required screenshot mapping

S01 environment, full SRN prompt, working directory; S02 GCC/Make versions; S03 project files; S04 clean build; S05 help; S06 FCFS; S07 RR and quantum; S08 priority and tie rule; S09 same-set policy comparison; S10 logger events/log; S11 orderly producer/consumer termination; S12 shared-memory exchange; S13 semaphore and IPC cleanup; S14 safe Banker sequence; S15 unsafe Banker state; S16 normal worker; S17 simulated failure detection; S18 resource limit and effect; S19 invalid input; S20 process/thread/IPC cleanup; S21 integrated demo; S22 log file contents; S23 final source, Makefile, executable; S24 ZIP contents and integrity.

The project handout also requires a team of exactly two and says the course calendar contains the deadline. Confirm the submission portal's ZIP requirement and naming rules before submission.
