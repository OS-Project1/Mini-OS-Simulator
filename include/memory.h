#ifndef MEMORY_H
#define MEMORY_H

#include "process.h"
#include "mm_config.h"

#include <stdbool.h>

void memory_init(void);

/** Invalidate all PTEs for this process (call once per process before run). */
void process_page_table_init(int pid);

/**
 * Translate and access virtual memory for process *p.
 * On success (hit): leaves *p unchanged, returns true.
 * On page fault: loads page (possibly with FIFO replacement), sets state BLOCKED
 * and blocked_ticks, logs; returns false.
 */
bool access_memory(Process *p, int virtual_address);

#endif
