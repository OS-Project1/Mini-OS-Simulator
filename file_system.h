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

/** Initialize an empty in-memory file system. */
void file_system_init(FileSystem *fs);

FsResult create_file(FileSystem *fs, const char *name);
FsResult delete_file(FileSystem *fs, const char *name);
FsResult write_file(FileSystem *fs, const char *name, const void *data, size_t size);

/**
 * Read a file's content (owned by the file system; do not free).
 * If out_size is non-NULL, it receives the byte size (excluding the trailing '\0').
 */
const char *read_file(const FileSystem *fs, const char *name, size_t *out_size);

/** Print a simple file listing to the given stream (stdout if out is NULL). */
void list_files(const FileSystem *fs, FILE *out);

#endif

