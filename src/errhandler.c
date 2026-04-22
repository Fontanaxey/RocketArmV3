#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include "errhandler.h"

static void get_absolute_log_path(char *dest, size_t dest_size, const char *filename)
{
    char exe_path[4096];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len != -1)
    {
        exe_path[len] = '\0';
        char *exe_dir = dirname(exe_path);
        snprintf(dest, dest_size, "%s/../logs/%s", exe_dir, filename);
    }
    else
        snprintf(dest, dest_size, "logs/%s", filename);
}

void log_event(LogType type, const char *msg)
{
    char final_path[4096];
    const char *filename;
    const char *level_str;

    switch (type)
    {
    case ACCESS:
        filename = "access_logs.txt";
        level_str = "ACCESS";
        break;
    case WARNING:
        filename = "error_logs.txt";
        level_str = "WARN";
        break;
    case FATAL:
        filename = "error_logs.txt";
        level_str = "FATAL";
        break;
    default:
        return;
    }

    get_absolute_log_path(final_path, sizeof(final_path), filename);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

    FILE *file = fopen(final_path, "a");
    if (file == NULL)
    {
        fprintf(stderr, "[%s] [CRIT] Cannot open log %s: %s\n", timestamp, final_path, strerror(errno));
        return;
    }

    fprintf(file, "[%s] [%s]: %s\n", timestamp, level_str, msg);
    fclose(file);
}