#ifndef USERS_HANDLER_H
#define USERS_HANDLER_H

#include <stdint.h>
#include <sqlite3.h>
#include "err_handler.h"

typedef enum
{
    USER_ROLE_ADMIN,
    USER_ROLE_OPERATOR,
    USER_ROLE_VIEWER,
    USER_ROLE_INVALID
} UserRole;

typedef enum
{
    USER_SUCCESS = 0,
    USER_ERR_INVALID_PARAM,
    USER_ERR_DUPLICATE,
    USER_ERR_DB
} UserStatus;

UserStatus user_create(sqlite3 *db, const char *username, const char *password_hash, UserRole role, int64_t *out_id);

UserRole user_string_to_role(const char *role_str);

UserStatus user_authenticate(sqlite3 *db, const char *username, const char *password, UserRole *out_role);

const char *user_role_to_string(UserRole role);

#endif