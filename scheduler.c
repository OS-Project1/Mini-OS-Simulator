#include "scheduler.h"

#include <stdio.h>

static int min_int(int a, int b)
{
    return a < b ? a : b;
}

void ready_queue_init(ReadyQueue *q)
{
    q->front = 0;
    q->rear = 0;
    q->count = 0;
}

int ready_queue_is_empty(const ReadyQueue *q)
{
    return q->count == 0;
}

int ready_queue_is_full(const ReadyQueue *q)
{
    return q->count == READY_QUEUE_MAX;
}

void ready_queue_enqueue(ReadyQueue *q, Process p)
{
    if (ready_queue_is_full(q)) {
        fprintf(stderr, "ready queue is full (max %d)\n", READY_QUEUE_MAX);
        return;
    }

    q->items[q->rear] = p;
    q->rear = (q->rear + 1) % READY_QUEUE_MAX;
    q->count++;
}

Process ready_queue_dequeue(ReadyQueue *q)
{
    Process empty = {0};

    if (ready_queue_is_empty(q)) {
        return empty;
    }

    Process p = q->items[q->front];
    q->front = (q->front + 1) % READY_QUEUE_MAX;
    q->count--;
    return p;
}

void round_robin(ReadyQueue *q, int time_quantum)
{
    int prev_pid = -1;

    if (time_quantum < 1) {
        time_quantum = 1;
    }

    while (!ready_queue_is_empty(q)) {
        Process p = ready_queue_dequeue(q);

        if (prev_pid != -1 && prev_pid != p.pid) {
            printf("context switch: pid %d -> pid %d\n", prev_pid, p.pid);
        }

        p.state = RUNNING;
        printf("running pid=%d\n", p.pid);

        {
            int slice = min_int(time_quantum, p.remaining_time);
            p.remaining_time -= slice;
        }

        if (p.remaining_time > 0) {
            p.state = READY;
            ready_queue_enqueue(q, p);
        } else {
            p.state = TERMINATED;
        }

        print_process(p);
        prev_pid = p.pid;
    }
}
