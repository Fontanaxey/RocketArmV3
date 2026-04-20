#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include "errhandler.h"

void log_event(LogType type, const char *msg)
{
    const char *filepath;
    const char *level_str;

    switch (type)
    {
    case ACCESS:
        filepath = "logs/access_logs";
        level_str = "ACCESS";
        break;
    case WARNING:
        filepath = "logs/error_log";
        level_str = "WARN";
        break;
    case FATAL:
        filepath = "logs/error_log";
        level_str = "FATAL";
        break;
    default:
        return;
    }

    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    if (timestamp)
        timestamp[24] = '\0';

    FILE *file = fopen(filepath, "a");
    if (file == NULL)
    {
        fprintf(stderr, "[%s] [SYS_CRIT] Failed I/O on %s: %s\n", timestamp ? timestamp : "UNKNOWN", filepath, strerror(errno));
        fprintf(stderr, "[%s] [%s]: %s\n", timestamp ? timestamp : "UNKNOWN", level_str, msg);
        return;
    }

    if (!strcmp("ACCESS", level_str))
        fprintf(file, "[%s]: %s logged in as\n", timestamp, msg);
    else
        fprintf(file, "[%s] [%s]: %s\n", timestamp, level_str, msg);

    fclose(file);
}