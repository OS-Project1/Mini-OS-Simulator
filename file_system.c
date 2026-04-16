#include "file_system.h"
#include "logging.h"

#include <stdlib.h>
#include <string.h>

static Logger g_logger;
static int g_logger_initialized = 0;

static FileSystem g_fs;
static int g_fs_initialized = 0;

static void fs_ensure_logger_init(void)
{
    if (!g_logger_initialized)
    {
        logging_init(&g_logger);
        g_logger_initialized = 1;
    }
}

static void fs_ensure_init(void)
{
    if (!g_fs_initialized)
    {
        g_fs.file_count = 0;
        for (size_t i = 0; i < FS_MAX_FILES; i++)
        {
            g_fs.files[i].name[0] = '\0';
            g_fs.files[i].size = 0;
            g_fs.files[i].content = NULL;
            g_fs.files[i].created_time = (time_t)0;
        }
        g_fs_initialized = 1;
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

void file_system_init(void)
{
    fs_ensure_init();
}

FsResult create_file(int pid, const char *name)
{
    if (!name || name[0] == '\0')
    {
        return FS_ERR_INVALID_ARG;
    }

    fs_ensure_init();
    fs_ensure_logger_init();
    logging_log(&g_logger, pid, LOG_OP_CREATE, name);

    size_t name_len = strnlen(name, FS_MAX_NAME);
    if (name_len >= FS_MAX_NAME)
    {
        return FS_ERR_NAME_TOO_LONG;
    }

    if (fs_find_index(&g_fs, name) >= 0)
    {
        return FS_ERR_EXISTS;
    }

    if (g_fs.file_count >= FS_MAX_FILES)
    {
        return FS_ERR_NO_SPACE;
    }

    int slot = fs_find_free_slot(&g_fs);
    if (slot < 0)
    {
        return FS_ERR_NO_SPACE;
    }

    File *f = &g_fs.files[slot];
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

    g_fs.file_count++;
    return FS_OK;
}

FsResult delete_file(int pid, const char *name)
{
    if (!name || name[0] == '\0')
    {
        return FS_ERR_INVALID_ARG;
    }

    fs_ensure_init();
    fs_ensure_logger_init();
    logging_log(&g_logger, pid, LOG_OP_DELETE, name);

    int idx = fs_find_index(&g_fs, name);
    if (idx < 0)
    {
        return FS_ERR_NOT_FOUND;
    }

    fs_clear_file(&g_fs.files[idx]);
    if (g_fs.file_count > 0)
    {
        g_fs.file_count--;
    }
    return FS_OK;
}

FsResult write_file(int pid, const char *name, const char *content)
{
    if (!name || name[0] == '\0')
    {
        return FS_ERR_INVALID_ARG;
    }
    if (!content)
    {
        return FS_ERR_INVALID_ARG;
    }

    fs_ensure_init();
    fs_ensure_logger_init();
    logging_log(&g_logger, pid, LOG_OP_WRITE, name);

    int idx = fs_find_index(&g_fs, name);
    if (idx < 0)
    {
        return FS_ERR_NOT_FOUND;
    }

    size_t size = strlen(content);
    File *f = &g_fs.files[idx];
    char *new_buf = (char *)realloc(f->content, size + 1);
    if (!new_buf)
    {
        return FS_ERR_NO_MEMORY;
    }

    memcpy(new_buf, content, size);
    new_buf[size] = '\0';

    f->content = new_buf;
    f->size = size;
    return FS_OK;
}

const char *read_file(int pid, const char *name)
{
    if (!name || name[0] == '\0')
    {
        return NULL;
    }

    fs_ensure_init();
    fs_ensure_logger_init();
    logging_log(&g_logger, pid, LOG_OP_READ, name);

    int idx = fs_find_index(&g_fs, name);
    if (idx < 0)
    {
        return NULL;
    }

    const File *f = &g_fs.files[idx];
    return f->content;
}

void list_files(FILE *out)
{
    if (!out)
    {
        out = stdout;
    }

    fs_ensure_init();

    fprintf(out, "Files (%zu/%d):\n", g_fs.file_count, FS_MAX_FILES);
    for (size_t i = 0; i < FS_MAX_FILES; i++)
    {
        const File *f = &g_fs.files[i];
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

