#ifndef PROCESS_H
#define PROCESS_H

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
} ProcessState;

typedef struct {
    int pid;
    int burst_time;
    int remaining_time;
    ProcessState state;
    /** Remaining simulated wait ticks while BLOCKED (I/O or memory load). */
    int blocked_ticks;
} Process;

void print_process(Process p);

#endif
