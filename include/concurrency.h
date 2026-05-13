#ifndef CONCURRENCY_H
#define CONCURRENCY_H

/*
 * Runs two scenarios:
 * 1) Baseline shared-counter update without synchronization.
 * 2) Enhanced version with mutex-based critical section.
 */
void concurrency_demo_run(void);

#endif
