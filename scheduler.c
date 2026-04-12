#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>

static int min_int(int a, int b)
{
    return a < b ? a : b;
}

static Process blocked_procs[READY_QUEUE_MAX];
static int blocked_io_ticks[READY_QUEUE_MAX];
static int blocked_count;

static void blocked_reset(void)
{
    blocked_count = 0;
}

static int blocked_is_empty(void)
{
    return blocked_count == 0;
}

static void blocked_add(Process p, int ticks)
{
    if (blocked_count >= READY_QUEUE_MAX) {
        fprintf(stderr, "blocked list is full (max %d)\n", READY_QUEUE_MAX);
        return;
    }

    blocked_procs[blocked_count] = p;
    blocked_io_ticks[blocked_count] = ticks;
    blocked_count++;
}

static void blocked_tick_and_wake(ReadyQueue *q)
{
    int i = 0;

    while (i < blocked_count) {
        blocked_io_ticks[i]--;

        if (blocked_io_ticks[i] <= 0) {
            Process p = blocked_procs[i];
            p.state = READY;
            printf("[I/O] Process P%d completed I/O → READY\n", p.pid);
            ready_queue_enqueue(q, p);

            blocked_count--;
            if (i < blocked_count) {
                blocked_procs[i] = blocked_procs[blocked_count];
                blocked_io_ticks[i] = blocked_io_ticks[blocked_count];
            }
        } else {
            i++;
        }
    }
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
    if (time_quantum < 1) {
        time_quantum = 1;
    }

    blocked_reset();

    while (!ready_queue_is_empty(q) || !blocked_is_empty()) {
        if (!ready_queue_is_empty(q)) {
            Process p = ready_queue_dequeue(q);

            printf("[Scheduler] Context switch to P%d\n", p.pid);

            p.state = RUNNING;
            printf("[Scheduler] Running P%d\n", p.pid);

            if (p.remaining_time > 0 && (rand() % 3 == 0)) {
                p.state = BLOCKED;
                printf("[I/O] Process P%d is blocked for I/O\n", p.pid);
                blocked_add(p, 1 + rand() % 3);
                continue;
            }

            {
                int slice = min_int(time_quantum, p.remaining_time);
                p.remaining_time -= slice;
            }

            if (p.remaining_time > 0) {
                p.state = READY;
                ready_queue_enqueue(q, p);
                printf("[Scheduler] Time slice over for P%d, re-queued\n", p.pid);
            } else {
                p.state = TERMINATED;
                printf("[Scheduler] P%d finished execution\n", p.pid);
            }
        } else {
            blocked_tick_and_wake(q);
        }
    }
}
