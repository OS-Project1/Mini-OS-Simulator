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
} Process;

void print_process(Process p);

#endif
