# CampusCompute

CampusCompute is a Linux/Ubuntu command-line mini-project for operating systems. It implements CPU scheduling, a POSIX-threaded bounded logger, shared-memory IPC coordinated by a named semaphore, Banker's safety analysis, worker process status collection, and a resource-limit demonstration.

## Requirements

- Linux/Ubuntu or WSL Ubuntu
- GCC and GNU Make
- POSIX threads, shared memory, named semaphores, `fork`, `waitpid`, and `setrlimit`

## Set up on Windows with WSL

Open **PowerShell as Administrator** and install WSL with Ubuntu:

```powershell
wsl --install -d Ubuntu
```

Restart Windows if prompted. Launch **Ubuntu** from the Start menu and create the Linux username/password on first launch. In Ubuntu, install the build tools, then clone this repository into the Linux home directory:

```sh
sudo apt update
sudo apt install -y build-essential git zip unzip
cd ~
git clone https://github.com/Vanshdev3/CampusCompute.git CampusCompute
cd ~/CampusCompute
```

The assignment requires an SRN prompt for terminal evidence. Set the submitting/capturing student's full actual SRN locally in that Ubuntu shell; do not use the example placeholder:

```sh
export SRN='YOUR_ACTUAL_FULL_SRN'
export PS1='[$SRN] \w\$ '
```

Keep using the same Ubuntu shell so this prompt remains active. The source changes are shared through GitHub; each teammate should capture screenshots on their own terminal with their own full SRN visible. One teammate submits the team's files.

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
./campuscompute cleanup
./campuscompute demo
```

Use lower numeric priority as higher priority. Scheduling ties use earlier arrival time, then lower job ID; FCFS uses arrival then ID. RR defaults to a quantum of 2. The same deterministic sample set is used throughout: J1=(arrival 0, burst 5, priority 2), J2=(1,3,1), J3=(2,8,3), J4=(3,6,1). Turnaround = completion - arrival; waiting = turnaround - burst.

`banker safe` and `banker unsafe` use the classic five-process, three-resource allocation/maxima sample; the unsafe run sets Available to (0,0,0). `worker fail` exits with status 7 by design and is explicitly labeled as a simulated failure. `limit` reduces the current process soft `RLIMIT_NOFILE` to at most 32, opens `/dev/null` descriptors until the kernel rejects another open, reports the observed effect, and closes the descriptors it opened. Run it as a standalone command because the limit applies to that process and its descendants.

The logger replaces the selected log file. IPC names include the parent PID and the program unlinks only the names it created. `make clean` removes object files, the executable, and the default log file.

## Evidence PDF and submission

Capture genuine screenshots in Ubuntu/WSL after building and running each command. Before capture, set the prompt with the submitting student's full actual SRN, for example:

```sh
export SRN='YOUR_ACTUAL_FULL_SRN'
export PS1='[$SRN] \w\$ '
```

Do not leave the placeholder in the final evidence. Screenshots must show the command and useful output; number them S01 through S24 and add one short explanatory sentence beneath each. Combine them in one PDF named only with the submitting student's SRN (`ACTUALSRN.pdf`). Do not submit a separate report. If the course portal requires a ZIP, include the source/build files and verify the archive with `unzip -t`. Follow any portal-specific directions. Both team members should understand and be ready to explain the implementation for the viva.

## Screenshot commands and captions

Capture the Ubuntu terminal directly. Every screenshot must show the shell command and full actual SRN prompt. Save the images as `S01.png` through `S24.png`; put the matching one-sentence caption under each screenshot in one PDF. Run the commands from `~/CampusCompute` unless the row changes directory.

| ID | Command to run | Required visible evidence | Caption |
|---|---|---|---|
| S01 | `pwd` | Ubuntu prompt with full SRN and project working directory. | S01 – Environment: Ubuntu shows my full SRN prompt and project directory. |
| S02 | `gcc --version && make --version` | GCC and Make version output. | S02 – Build tools: GCC and Make are available. |
| S03 | `find . -maxdepth 2 -type f -not -path './.git/*' -printf '%p\n'` | README, Makefile, and all source/header files. | S03 – Project files: The source, Makefile, and README are present. |
| S04 | `make clean && make` | Clean build and successful compilation/linking. | S04 – Clean build: CampusCompute builds from source. |
| S05 | `./campuscompute help` | All documented independent CLI commands. | S05 – CLI help: The help lists the available project features. |
| S06 | `./campuscompute schedule fcfs` | FCFS timeline, tie-break, status, completion/turnaround/waiting metrics and averages. | S06 – FCFS: FCFS reports its timeline and scheduling metrics. |
| S07 | `./campuscompute schedule rr 2` | Quantum, FIFO rule, interleaved timeline, status and metrics. | S07 – Round Robin: Round Robin rotates jobs with quantum 2 and reports metrics. |
| S08 | `./campuscompute schedule priority` | Priorities, priority tie-break, timeline, status and metrics. | S08 – Priority: Non-preemptive priority scheduling reports its rule and metrics. |
| S09 | `./campuscompute compare` | All three policies and summary metrics using the same sample jobs. | S09 – Comparison: The policies use the same repeatable job set. |
| S10 | `./campuscompute logger 8 campuscompute.log` | Producer queue activity and consumer-written events; log file is generated. | S10 – Producer-consumer logger: Threads exchange events and write the log. |
| S11 | `./campuscompute logger 3 campuscompute.log` | Producer and consumer join messages and clean logger termination. | S11 – Thread termination: Both logger threads are joined cleanly. |
| S12 | `./campuscompute ipc` | Parent reads a message written by the child into shared memory. | S12 – Shared memory: The child and parent exchange data through shared memory. |
| S13 | `./campuscompute ipc` | Semaphore signal confirmation and created IPC cleanup message. | S13 – Semaphore cleanup: Semaphore coordination completes and created IPC resources are released. |
| S14 | `./campuscompute banker safe` | Available, Allocation, Maximum, Need, and a valid safe sequence. | S14 – Safe state: Banker’s algorithm finds a valid safe sequence. |
| S15 | `./campuscompute banker unsafe` | Resource matrices and unsafe result/processes unable to finish. | S15 – Unsafe state: Banker’s algorithm identifies the unsafe state. |
| S16 | `./campuscompute worker normal` | Child PID and parent-reported normal exit status. | S16 – Normal worker: The parent collects the child's normal completion status. |
| S17 | `./campuscompute worker fail` | Clearly labeled simulated failure and parent detection of non-zero status 7. | S17 – Simulated failure: The parent detects the controlled worker failure. |
| S18 | `./campuscompute limit` | Applied/observed descriptor limit and an `EMFILE` failure after opening descriptors. | S18 – Resource limit: The process limit produces an observed open failure. |
| S19 | `./campuscompute invalid` | Useful invalid-command and usage output. | S19 – Error handling: An invalid command displays an error and usage. |
| S20 | `./campuscompute cleanup` | Logger joins, both workers are reaped, IPC cleanup message, and no matching names remain in `/dev/shm`. | S20 – Cleanup verification: Threads and children finish, and created IPC names are released. |
| S21 | `./campuscompute demo` | Integrated run and final summary. Maximize terminal so command and summary are visible; use terminal scrollback if needed. | S21 – Integrated run: The integrated demonstration runs and summarizes all features. |
| S22 | `cat campuscompute.log` | Timestamped generated log entries. | S22 – Generated log: The file contains events written by the consumer thread. |
| S23 | `ls -l src Makefile README.md campuscompute` | Final source/header files, Makefile, README, and executable. | S23 – Final files: Source and build files are present with the executable. |
| S24 | `cd .. && zip -r CampusCompute.zip CampusCompute/.gitignore CampusCompute/Makefile CampusCompute/README.md CampusCompute/src && unzip -l CampusCompute.zip && unzip -t CampusCompute.zip` | ZIP contents and successful archive integrity check. | S24 – ZIP check: The archive contains source/build files and passes integrity checking. |

For S20, an empty search result is represented by the final “No CampusCompute IPC objects remain” line. Do not claim more than that check verifies. Keep S21's output genuine; do not edit or fabricate terminal output.

The project handout also requires a team of exactly two and says the course calendar contains the deadline. Confirm the submission portal's ZIP requirement and naming rules before submission.
