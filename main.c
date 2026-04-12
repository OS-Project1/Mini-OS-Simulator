#include "process.h"
#include "scheduler.h"
#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    srand((unsigned)time(NULL));

    memory_init();
    process_page_table_init(1);
    process_page_table_init(2);
    process_page_table_init(3);

    ReadyQueue queue;
    ready_queue_init(&queue);

    Process p1 = {.pid = 1, .burst_time = 10, .remaining_time = 10, .state = READY, .blocked_ticks = 0};
    Process p2 = {.pid = 2, .burst_time = 5, .remaining_time = 5, .state = READY, .blocked_ticks = 0};
    Process p3 = {.pid = 3, .burst_time = 8, .remaining_time = 8, .state = READY, .blocked_ticks = 0};

    ready_queue_enqueue(&queue, p1);
    ready_queue_enqueue(&queue, p2);
    ready_queue_enqueue(&queue, p3);

    round_robin(&queue, 2);

    return 0;
}
