#include "process.h"

#include <stdio.h>

void print_process(Process p)
{
    const char *state_str;

    switch (p.state) {
    case READY:
        state_str = "READY";
        break;
    case RUNNING:
        state_str = "RUNNING";
        break;
    case BLOCKED:
        state_str = "BLOCKED";
        break;
    case TERMINATED:
        state_str = "TERMINATED";
        break;
    default:
        state_str = "UNKNOWN";
        break;
    }

    printf("pid=%d state=%s remaining_time=%d\n", p.pid, state_str, p.remaining_time);
}
