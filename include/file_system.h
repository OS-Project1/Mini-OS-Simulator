#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stddef.h>
#include <stdio.h>
#include <time.h>

#ifndef FS_MAX_FILES
#define FS_MAX_FILES 100
#endif

#ifndef FS_MAX_NAME
#define FS_MAX_NAME 64
#endif

typedef struct File
{
    char name[FS_MAX_NAME];
    size_t size;
    char *content;
    time_t created_time;
} File;

typedef struct FileSystem
{
    File files[FS_MAX_FILES];
    size_t file_count;
} FileSystem;

typedef enum FsResult
{
    FS_OK = 0,
    FS_ERR_INVALID_ARG = -1,
    FS_ERR_NAME_TOO_LONG = -2,
    FS_ERR_EXISTS = -3,
    FS_ERR_NOT_FOUND = -4,
    FS_ERR_NO_SPACE = -5,
    FS_ERR_NO_MEMORY = -6
} FsResult;

/** Initialize the internal in-memory file system (optional; auto-inits on first use). */
void file_system_init(void);

FsResult create_file(int pid, const char *name);
FsResult delete_file(int pid, const char *name);
FsResult write_file(int pid, const char *name, const char *content);

/**
 * Read a file's content (owned by the file system; do not free).
 */
const char *read_file(int pid, const char *name);

/** Print a simple file listing to the given stream (stdout if out is NULL). */
void list_files(FILE *out);

#endif

