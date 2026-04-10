#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

#define READY_QUEUE_MAX 10

typedef struct {
    Process items[READY_QUEUE_MAX];
    int front;
    int rear;
    int count;
} ReadyQueue;

void ready_queue_init(ReadyQueue *q);
void ready_queue_enqueue(ReadyQueue *q, Process p);
int ready_queue_is_empty(const ReadyQueue *q);
int ready_queue_is_full(const ReadyQueue *q);
Process ready_queue_dequeue(ReadyQueue *q);

void round_robin(ReadyQueue *q, int time_quantum);

#endif
