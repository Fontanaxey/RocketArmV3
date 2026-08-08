#ifndef ERRHANDLER_H
#define ERRHANDLER_H

#include <stddef.h>

typedef enum
{
    LOG_INFO,             // ordinary events                    (app.log)
    LOG_WARN,             // warnings                           (app.log)
    LOG_ERROR,            // blocking errors                    (error.log)
    LOG_FATAL,            // critical system errors             (error.log)
    LOG_SECURITY_SUCCESS, // successful login/access            (audit.log)
    LOG_SECURITY_FAIL,    // failed intrusion/login attempts    (audit.log)
    LOG_AUDIT             // user actions tracked               (audit.log)
} LogType;

void log_event(LogType type, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

#endif