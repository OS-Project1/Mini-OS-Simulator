#include "process.h"
#include "scheduler.h"

#include <stdio.h>

int main(void)
{
    ReadyQueue rq;
    ready_queue_init(&rq);

    Process p1 = {.pid = 1, .burst_time = 10, .remaining_time = 10, .state = READY};
    Process p2 = {.pid = 2, .burst_time = 5, .remaining_time = 5, .state = READY};
    Process p3 = {.pid = 3, .burst_time = 8, .remaining_time = 8, .state = READY};

    ready_queue_enqueue(&rq, p1);
    ready_queue_enqueue(&rq, p2);
    ready_queue_enqueue(&rq, p3);

    round_robin(&rq, 2);

    return 0;
}
