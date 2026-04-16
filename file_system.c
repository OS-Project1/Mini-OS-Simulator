#include "file_system.h"
#include "logging.h"

#include <stdlib.h>
#include <string.h>

static Logger g_logger;
static int g_logger_initialized = 0;

static void fs_ensure_logger_init(void)
{
    if (!g_logger_initialized)
    {
        logging_init(&g_logger);
        g_logger_initialized = 1;
    }
}

static int fs_find_index(const FileSystem *fs, const char *name)
{
    if (!fs || !name)
    {
        return -1;
    }

    for (size_t i = 0; i < FS_MAX_FILES; i++)
    {
        if (fs->files[i].name[0] == '\0')
        {
            continue;
        }
        if (strncmp(fs->files[i].name, name, FS_MAX_NAME) == 0)
        {
            return (int)i;
        }
    }

    return -1;
}

static int fs_find_free_slot(const FileSystem *fs)
{
    if (!fs)
    {
        return -1;
    }

    for (size_t i = 0; i < FS_MAX_FILES; i++)
    {
        if (fs->files[i].name[0] == '\0')
        {
            return (int)i;
        }
    }
    return -1;
}

static void fs_clear_file(File *f)
{
    if (!f)
    {
        return;
    }

    free(f->content);
    f->content = NULL;
    f->size = 0;
    f->created_time = (time_t)0;
    f->name[0] = '\0';
}

void file_system_init(FileSystem *fs)
{
    if (!fs)
    {
        return;
    }

    fs->file_count = 0;
    for (size_t i = 0; i < FS_MAX_FILES; i++)
    {
        fs->files[i].name[0] = '\0';
        fs->files[i].size = 0;
        fs->files[i].content = NULL;
        fs->files[i].created_time = (time_t)0;
    }
}

FsResult create_file(FileSystem *fs, const char *name)
{
    if (!fs || !name || name[0] == '\0')
    {
        return FS_ERR_INVALID_ARG;
    }

    fs_ensure_logger_init();
    logging_log(&g_logger, LOG_OP_CREATE, name);

    size_t name_len = strnlen(name, FS_MAX_NAME);
    if (name_len >= FS_MAX_NAME)
    {
        return FS_ERR_NAME_TOO_LONG;
    }

    if (fs_find_index(fs, name) >= 0)
    {
        return FS_ERR_EXISTS;
    }

    if (fs->file_count >= FS_MAX_FILES)
    {
        return FS_ERR_NO_SPACE;
    }

    int slot = fs_find_free_slot(fs);
    if (slot < 0)
    {
        return FS_ERR_NO_SPACE;
    }

    File *f = &fs->files[slot];
    memcpy(f->name, name, name_len);
    f->name[name_len] = '\0';
    f->size = 0;
    f->created_time = time(NULL);

    f->content = (char *)malloc(1);
    if (!f->content)
    {
        fs_clear_file(f);
        return FS_ERR_NO_MEMORY;
    }
    f->content[0] = '\0';

    fs->file_count++;
    return FS_OK;
}

FsResult delete_file(FileSystem *fs, const char *name)
{
    if (!fs || !name || name[0] == '\0')
    {
        return FS_ERR_INVALID_ARG;
    }

    fs_ensure_logger_init();
    logging_log(&g_logger, LOG_OP_DELETE, name);

    int idx = fs_find_index(fs, name);
    if (idx < 0)
    {
        return FS_ERR_NOT_FOUND;
    }

    fs_clear_file(&fs->files[idx]);
    if (fs->file_count > 0)
    {
        fs->file_count--;
    }
    return FS_OK;
}

FsResult write_file(FileSystem *fs, const char *name, const void *data, size_t size)
{
    if (!fs || !name || name[0] == '\0')
    {
        return FS_ERR_INVALID_ARG;
    }
    if (size > 0 && !data)
    {
        return FS_ERR_INVALID_ARG;
    }

    fs_ensure_logger_init();
    logging_log(&g_logger, LOG_OP_WRITE, name);

    int idx = fs_find_index(fs, name);
    if (idx < 0)
    {
        return FS_ERR_NOT_FOUND;
    }

    File *f = &fs->files[idx];
    char *new_buf = (char *)realloc(f->content, size + 1);
    if (!new_buf)
    {
        return FS_ERR_NO_MEMORY;
    }

    if (size > 0)
    {
        memcpy(new_buf, data, size);
    }
    new_buf[size] = '\0';

    f->content = new_buf;
    f->size = size;
    return FS_OK;
}

const char *read_file(const FileSystem *fs, const char *name, size_t *out_size)
{
    if (out_size)
    {
        *out_size = 0;
    }
    if (!fs || !name || name[0] == '\0')
    {
        return NULL;
    }

    fs_ensure_logger_init();
    logging_log(&g_logger, LOG_OP_READ, name);

    int idx = fs_find_index(fs, name);
    if (idx < 0)
    {
        return NULL;
    }

    const File *f = &fs->files[idx];
    if (out_size)
    {
        *out_size = f->size;
    }
    return f->content;
}

void list_files(const FileSystem *fs, FILE *out)
{
    if (!out)
    {
        out = stdout;
    }

    if (!fs)
    {
        fprintf(out, "(file system is NULL)\n");
        return;
    }

    fprintf(out, "Files (%zu/%d):\n", fs->file_count, FS_MAX_FILES);
    for (size_t i = 0; i < FS_MAX_FILES; i++)
    {
        const File *f = &fs->files[i];
        if (f->name[0] == '\0')
        {
            continue;
        }

        char ts[32] = {0};
        struct tm *tm_info = localtime(&f->created_time);
        if (tm_info)
        {
            strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tm_info);
        }
        else
        {
            strncpy(ts, "unknown", sizeof(ts) - 1);
        }

        fprintf(out, "- %-*s  size=%zu  created=%s\n", (int)(FS_MAX_NAME - 1), f->name, f->size, ts);
    }
}

