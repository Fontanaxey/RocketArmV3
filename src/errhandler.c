#include "err_handler.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>

#define MAX_LOG_PATH 4096
#define MAX_MSG_SIZE 2048

static void get_absolute_log_path(char *dest, size_t dest_size, const char *filename)
{
    char exe_path[MAX_LOG_PATH];
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

void log_event(LogType type, const char *fmt, ...)
{
    char final_path[MAX_LOG_PATH];
    char formatted_msg[MAX_MSG_SIZE];
    const char *filename;
    const char *level_str;

    // Routing dei log in base al tipo di evento
    switch (type)
    {
    case LOG_INFO:
        filename = "app.log";
        level_str = "INFO";
        break;
    case LOG_WARN:
        filename = "app.log";
        level_str = "WARN";
        break;
    case LOG_ERROR:
        filename = "error.log";
        level_str = "ERROR";
        break;
    case LOG_FATAL:
        filename = "error.log";
        level_str = "FATAL";
        break;
    case LOG_SECURITY_SUCCESS:
        filename = "audit.log";
        level_str = "AUTH_OK";
        break;
    case LOG_SECURITY_FAIL:
        filename = "audit.log";
        level_str = "AUTH_FAIL";
        break;
    case LOG_AUDIT:
        filename = "audit.log";
        level_str = "AUDIT";
        break;
    default:
        return;
    }

    // Formattazione variadica del messaggio
    va_list args;
    va_start(args, fmt);
    vsnprintf(formatted_msg, sizeof(formatted_msg), fmt, args);
    va_end(args);

    get_absolute_log_path(final_path, sizeof(final_path), filename);

    // Timestamp reentrante (thread-safe)
    time_t now = time(NULL);
    struct tm t;
    localtime_r(&now, &t);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &t);

    FILE *file = fopen(final_path, "a");
    if (file == NULL)
    {
        fprintf(stderr, "[%s] [CRIT] Impossibile aprire il file di log %s: %s\n",
                timestamp, final_path, strerror(errno));
        return;
    }

    fprintf(file, "[%s] [%s] %s\n", timestamp, level_str, formatted_msg);
    fflush(file); // Forza la scrittura immediata su disco
    fclose(file);
}