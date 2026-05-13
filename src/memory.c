#include "memory.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int frame;
    int valid;
} PageTableEntry;

typedef struct {
    PageTableEntry pt[MM_MAX_PAGES];
} PageTable;

typedef struct {
    int pid;
    int vpn;
    int in_use;
} FrameDesc;

static PageTable g_tables[MM_MAX_PID];
static FrameDesc g_frames[MM_FRAME_COUNT];

/* FIFO queue of frame indices (oldest at front). */
static int g_fifo[MM_FRAME_COUNT];
static int g_fifo_head;
static int g_fifo_tail;
static int g_fifo_count;

static void fifo_push(int frame_id)
{
    if (g_fifo_count >= MM_FRAME_COUNT) {
        return;
    }
    g_fifo[g_fifo_tail] = frame_id;
    g_fifo_tail = (g_fifo_tail + 1) % MM_FRAME_COUNT;
    g_fifo_count++;
}

static int fifo_pop(void)
{
    int f = g_fifo[g_fifo_head];
    g_fifo_head = (g_fifo_head + 1) % MM_FRAME_COUNT;
    g_fifo_count--;
    return f;
}

static PageTable *table_for_pid(int pid)
{
    if (pid <= 0 || pid >= MM_MAX_PID) {
        return NULL;
    }
    return &g_tables[pid];
}

static int find_free_frame(void)
{
    for (int i = 0; i < MM_FRAME_COUNT; i++) {
        if (!g_frames[i].in_use) {
            return i;
        }
    }
    return -1;
}

static void invalidate_pte(int pid, int vpn)
{
    PageTable *t = table_for_pid(pid);
    if (!t || vpn < 0 || vpn >= MM_MAX_PAGES) {
        return;
    }
    t->pt[vpn].valid = 0;
    t->pt[vpn].frame = -1;
}

/**
 * Bring virtual page `vpn` of `pid` into physical memory; may evict FIFO victim.
 */
static void load_page_into_memory(int pid, int vpn)
{
    int frame_id = find_free_frame();

    if (frame_id < 0) {
        frame_id = fifo_pop();
        int victim_pid = g_frames[frame_id].pid;
        int victim_vpn = g_frames[frame_id].vpn;
        printf("[Memory] Replacing page %d of Process %d (FIFO)\n", victim_vpn, victim_pid);
        invalidate_pte(victim_pid, victim_vpn);
    }

    g_frames[frame_id].pid = pid;
    g_frames[frame_id].vpn = vpn;
    g_frames[frame_id].in_use = 1;

    PageTable *t = table_for_pid(pid);
    if (t) {
        t->pt[vpn].valid = 1;
        t->pt[vpn].frame = frame_id;
    }

    printf("[Memory] Allocated frame %d to Process %d (page %d)\n", frame_id, pid, vpn);
    fifo_push(frame_id);
}

void memory_init(void)
{
    memset(g_tables, 0, sizeof(g_tables));
    memset(g_frames, 0, sizeof(g_frames));
    for (int i = 0; i < MM_FRAME_COUNT; i++) {
        g_frames[i].pid = -1;
        g_frames[i].vpn = -1;
    }
    g_fifo_head = 0;
    g_fifo_tail = 0;
    g_fifo_count = 0;
}

void process_page_table_init(int pid)
{
    PageTable *t = table_for_pid(pid);
    if (!t) {
        return;
    }
    for (int i = 0; i < MM_MAX_PAGES; i++) {
        t->pt[i].valid = 0;
        t->pt[i].frame = -1;
    }
}

bool access_memory(Process *p, int virtual_address)
{
    if (!p) {
        return false;
    }

    if (virtual_address < 0) {
        return false;
    }

    const int page = virtual_address / MM_PAGE_SIZE;
    (void)(virtual_address % MM_PAGE_SIZE);

    if (page < 0 || page >= MM_MAX_PAGES) {
        /* Out-of-range virtual address: do not block the process. */
        return false;
    }

    PageTable *t = table_for_pid(p->pid);
    if (!t) {
        return false;
    }

    if (t->pt[page].valid) {
        return true;
    }

    printf("[Memory] Page fault in Process %d at page %d\n", p->pid, page);

    p->state = BLOCKED;
    p->blocked_ticks = MM_LOAD_TICKS;

    load_page_into_memory(p->pid, page);

    return false;
}
