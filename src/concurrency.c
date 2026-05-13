#include "concurrency.h"

#include <stdio.h>

#define CONC_THREADS 3
#define CONC_INCREMENTS 4
#define WAIT_Q_MAX 16

typedef enum {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_DONE
} ThreadState;

typedef struct {
    int tid;
    int remaining_ops;
    int phase;
    int snapshot;
    ThreadState state;
} SimThread;

typedef struct {
    int locked;
    int owner_tid;
    int wait_q[WAIT_Q_MAX];
    int front;
    int rear;
    int count;
} SimMutex;

static const char *state_str(ThreadState st)
{
    switch (st) {
    case THREAD_READY:
        return "READY";
    case THREAD_RUNNING:
        return "RUNNING";
    case THREAD_BLOCKED:
        return "BLOCKED";
    case THREAD_DONE:
        return "DONE";
    default:
        return "UNKNOWN";
    }
}

static int all_done(SimThread threads[], int n)
{
    for (int i = 0; i < n; i++) {
        if (threads[i].state != THREAD_DONE) {
            return 0;
        }
    }
    return 1;
}

static void mutex_init(SimMutex *m)
{
    m->locked = 0;
    m->owner_tid = -1;
    m->front = 0;
    m->rear = 0;
    m->count = 0;
}

static int mutex_try_acquire(SimMutex *m, SimThread *t)
{
    if (!m->locked) {
        m->locked = 1;
        m->owner_tid = t->tid;
        printf("[Lock] T%d acquired mutex\n", t->tid);
        return 1;
    }

    if (m->count < WAIT_Q_MAX) {
        m->wait_q[m->rear] = t->tid;
        m->rear = (m->rear + 1) % WAIT_Q_MAX;
        m->count++;
    }

    t->state = THREAD_BLOCKED;
    printf("[Scheduler] T%d blocked on mutex (owner=T%d)\n", t->tid, m->owner_tid);
    return 0;
}

static int mutex_release(SimMutex *m)
{
    int next_tid = -1;

    if (!m->locked) {
        return next_tid;
    }

    printf("[Lock] T%d released mutex\n", m->owner_tid);
    m->locked = 0;
    m->owner_tid = -1;

    if (m->count > 0) {
        next_tid = m->wait_q[m->front];
        m->front = (m->front + 1) % WAIT_Q_MAX;
        m->count--;
    }

    return next_tid;
}

static void run_baseline_no_lock(void)
{
    SimThread threads[CONC_THREADS];
    int shared_counter = 0;
    int tick = 0;

    for (int i = 0; i < CONC_THREADS; i++) {
        threads[i].tid = i + 1;
        threads[i].remaining_ops = CONC_INCREMENTS;
        threads[i].phase = 0;
        threads[i].snapshot = 0;
        threads[i].state = THREAD_READY;
    }

    printf("\n=== [Concurrency] Baseline: No lock ===\n");
    while (!all_done(threads, CONC_THREADS)) {
        SimThread *t = &threads[tick % CONC_THREADS];
        tick++;

        if (t->state == THREAD_DONE) {
            continue;
        }

        t->state = THREAD_RUNNING;
        printf("[Scheduler] Dispatch T%d (%s)\n", t->tid, state_str(t->state));

        if (t->phase == 0) {
            t->snapshot = shared_counter;
            printf("[Thread T%d] read shared=%d\n", t->tid, t->snapshot);
            t->phase = 1;
        } else if (t->phase == 1) {
            t->snapshot = t->snapshot + 1;
            printf("[Thread T%d] local increment -> %d\n", t->tid, t->snapshot);
            t->phase = 2;
        } else {
            shared_counter = t->snapshot;
            t->remaining_ops--;
            printf("[Thread T%d] write shared=%d (remaining_ops=%d)\n",
                   t->tid, shared_counter, t->remaining_ops);
            t->phase = 0;
        }

        if (t->remaining_ops <= 0) {
            t->state = THREAD_DONE;
            printf("[Scheduler] T%d terminated\n", t->tid);
        } else {
            t->state = THREAD_READY;
        }
    }

    printf("[Result] Baseline expected=%d, actual=%d\n",
           CONC_THREADS * CONC_INCREMENTS, shared_counter);
}

static void run_enhanced_with_mutex(void)
{
    SimThread threads[CONC_THREADS];
    SimMutex mutex;
    int shared_counter = 0;
    int tick = 0;

    mutex_init(&mutex);
    for (int i = 0; i < CONC_THREADS; i++) {
        threads[i].tid = i + 1;
        threads[i].remaining_ops = CONC_INCREMENTS;
        threads[i].phase = 0;
        threads[i].snapshot = 0;
        threads[i].state = THREAD_READY;
    }

    printf("\n=== [Concurrency] Enhanced: Mutex-protected critical section ===\n");
    while (!all_done(threads, CONC_THREADS)) {
        SimThread *t = &threads[tick % CONC_THREADS];
        tick++;

        if (t->state == THREAD_DONE) {
            continue;
        }

        if (t->state == THREAD_BLOCKED) {
            continue;
        }

        t->state = THREAD_RUNNING;
        printf("[Scheduler] Dispatch T%d (%s)\n", t->tid, state_str(t->state));

        if (t->phase == 0) {
            if (!mutex_try_acquire(&mutex, t)) {
                continue;
            }
            /*
             * Keep the lock across ticks so other threads get dispatched,
             * attempt acquire, and transition to BLOCKED.
             */
            t->phase = 1;
            t->state = THREAD_READY;
            continue;
        }

        if (t->phase == 1) {
            int old = shared_counter;
            shared_counter = old + 1;
            printf("[Critical] T%d: %d -> %d (remaining_ops=%d)\n",
                   t->tid, old, shared_counter, t->remaining_ops - 1);
            t->phase = 2;
            t->state = THREAD_READY;
            continue;
        }

        if (t->phase == 2) {
            {
                int wake_tid = mutex_release(&mutex);
                if (wake_tid > 0) {
                    SimThread *w = &threads[wake_tid - 1];
                    w->state = THREAD_READY;
                    printf("[Scheduler] T%d unblocked and moved to READY\n", wake_tid);
                }
            }

            t->remaining_ops--;
            t->phase = 0;
        }

        if (t->remaining_ops <= 0) {
            t->state = THREAD_DONE;
            printf("[Scheduler] T%d terminated\n", t->tid);
        } else if (t->state != THREAD_BLOCKED) {
            t->state = THREAD_READY;
        }
    }

    printf("[Result] Enhanced expected=%d, actual=%d\n",
           CONC_THREADS * CONC_INCREMENTS, shared_counter);
}

void concurrency_demo_run(void)
{
    run_baseline_no_lock();
    run_enhanced_with_mutex();
}
