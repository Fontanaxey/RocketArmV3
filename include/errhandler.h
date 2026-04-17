#ifndef ERRHANDLER_H
#define ERRHANDLER_H

typedef enum
{
    ACCESS,
    WARNING,
    FATAL
} LogType;

void log_event(LogType type, const char *msg);

#endif