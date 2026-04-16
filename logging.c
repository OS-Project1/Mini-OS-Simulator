#include "logging.h"

#include <stdio.h>
#include <string.h>

static void logging_format_timestamp(time_t t, char *out, size_t out_len)
{
    if (!out || out_len == 0)
    {
        return;
    }

    out[0] = '\0';
    struct tm *tm_info = localtime(&t);
    if (!tm_info)
    {
        strncpy(out, "unknown", out_len - 1);
        out[out_len - 1] = '\0';
        return;
    }

    strftime(out, out_len, "%Y-%m-%d %H:%M:%S", tm_info);
}

const char *logging_operation_to_string(LogOperation op)
{
    switch (op)
    {
    case LOG_OP_CREATE:
        return "CREATE";
    case LOG_OP_DELETE:
        return "DELETE";
    case LOG_OP_READ:
        return "READ";
    case LOG_OP_WRITE:
        return "WRITE";
    default:
        return "UNKNOWN";
    }
}

void logging_init(Logger *logger)
{
    if (!logger)
    {
        return;
    }

    logger->count = 0;
    logger->next_index = 0;
    for (size_t i = 0; i < LOG_MAX_ENTRIES; i++)
    {
        logger->entries[i].timestamp = (time_t)0;
        logger->entries[i].operation = LOG_OP_READ;
        logger->entries[i].file_name[0] = '\0';
    }
}

void logging_log(Logger *logger, LogOperation op, const char *file_name)
{
    if (!logger || !file_name || file_name[0] == '\0')
    {
        return;
    }

    LogEntry *e = &logger->entries[logger->next_index];
    e->timestamp = time(NULL);
    e->operation = op;

    size_t len = strnlen(file_name, LOG_MAX_NAME);
    if (len >= LOG_MAX_NAME)
    {
        len = LOG_MAX_NAME - 1;
    }
    memcpy(e->file_name, file_name, len);
    e->file_name[len] = '\0';

    if (logger->count < LOG_MAX_ENTRIES)
    {
        logger->count++;
    }
    logger->next_index = (logger->next_index + 1) % LOG_MAX_ENTRIES;

    char ts[32];
    logging_format_timestamp(e->timestamp, ts, sizeof(ts));
    printf("[LOG %s] %-6s %s\n", ts, logging_operation_to_string(op), e->file_name);
}

size_t logging_count(const Logger *logger)
{
    return logger ? logger->count : 0;
}

const LogEntry *logging_get(const Logger *logger, size_t index)
{
    if (!logger)
    {
        return NULL;
    }
    if (index >= logger->count)
    {
        return NULL;
    }

    size_t start = 0;
    if (logger->count == LOG_MAX_ENTRIES)
    {
        start = logger->next_index;
    }

    size_t pos = (start + index) % LOG_MAX_ENTRIES;
    return &logger->entries[pos];
}

