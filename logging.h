#ifndef LOGGING_H
#define LOGGING_H

#include <stddef.h>
#include <time.h>

#ifndef LOG_MAX_ENTRIES
#define LOG_MAX_ENTRIES 100
#endif

#ifndef LOG_MAX_NAME
#define LOG_MAX_NAME 64
#endif

typedef enum LogOperation
{
    LOG_OP_CREATE = 0,
    LOG_OP_DELETE = 1,
    LOG_OP_READ = 2,
    LOG_OP_WRITE = 3
} LogOperation;

typedef struct LogEntry
{
    time_t timestamp;
    LogOperation operation;
    char file_name[LOG_MAX_NAME];
} LogEntry;

typedef struct Logger
{
    LogEntry entries[LOG_MAX_ENTRIES];
    size_t count;
    size_t next_index;
} Logger;

/** Initialize logger state (call once at program start). */
void logging_init(Logger *logger);

/** Log an operation: prints to console and stores in the ring buffer. */
void logging_log(Logger *logger, LogOperation op, const char *file_name);

/** Get the number of stored logs (<= LOG_MAX_ENTRIES). */
size_t logging_count(const Logger *logger);

/**
 * Get a stored log entry by index [0..count-1], oldest-to-newest.
 * Returns NULL if index is out of range.
 */
const LogEntry *logging_get(const Logger *logger, size_t index);

/** Human-readable operation string (e.g. "CREATE"). */
const char *logging_operation_to_string(LogOperation op);

#endif

