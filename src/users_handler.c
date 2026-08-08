#include <sqlite3.h>
#include <string.h>
#include <openssl/evp.h>

#include "users_handler.h"
#include "err_handler.h"

const char *user_role_to_string(UserRole role)
{
    switch (role)
    {
    case USER_ROLE_ADMIN:
        return "admin";
    case USER_ROLE_OPERATOR:
        return "operator";
    case USER_ROLE_VIEWER:
        return "viewer";
    default:
        return NULL;
    }
}

UserRole user_string_to_role(const char *role_str)
{
    if (!role_str)
        return USER_ROLE_INVALID;

    if (strcmp(role_str, "admin") == 0)
        return USER_ROLE_ADMIN;
    if (strcmp(role_str, "operator") == 0)
        return USER_ROLE_OPERATOR;
    if (strcmp(role_str, "viewer") == 0)
        return USER_ROLE_VIEWER;

    return USER_ROLE_INVALID;
}

UserStatus user_create(sqlite3 *db, const char *username, const char *password_hash, UserRole role, int64_t *out_id)
{
    if (!db || !username || username[0] == '\0' || !password_hash || password_hash[0] == '\0')
    {
        log_event(LOG_WARN, "user creation failed: invalid or empty input parameters.");
        return USER_ERR_INVALID_PARAM;
    }

    const char *role_str = user_role_to_string(role);
    if (!role_str)
    {
        log_event(LOG_WARN, "user creation failed: invalid role (%d) specified for '%s'.", role, username);
        return USER_ERR_INVALID_PARAM;
    }

    static const char *sql =
        "INSERT INTO users (username, password_hash, role) "
        "VALUES (?, ?, ?);";

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        log_event(LOG_ERROR, "query failed for user '%s': %s", username, sqlite3_errmsg(db));
        return USER_ERR_DB;
    }

    sqlite3_bind_text(stmt, 1, username, strlen(username), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password_hash, strlen(password_hash), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, role_str, strlen(role_str), SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        int ext_rc = sqlite3_extended_errcode(db);
        sqlite3_finalize(stmt);

        if (rc == SQLITE_CONSTRAINT && (ext_rc == SQLITE_CONSTRAINT_UNIQUE || ext_rc == SQLITE_CONSTRAINT_PRIMARYKEY))
        {
            log_event(LOG_WARN, "user creation failed: username '%s' already exists.", username);
            return USER_ERR_DUPLICATE;
        }

        log_event(LOG_ERROR, "DB error while inserting user '%s': %s (code: %d)",
                  username, sqlite3_errmsg(db), ext_rc);
        return USER_ERR_DB;
    }

    int64_t created_id = (int64_t)sqlite3_last_insert_rowid(db);
    if (out_id)
        *out_id = created_id;

    sqlite3_finalize(stmt);

    log_event(LOG_AUDIT, "User '%s' (ID: %lld, Role: %s) created successfully.",
              username, (long long)created_id, role_str);

    return USER_SUCCESS;
}

UserStatus user_authenticate(sqlite3 *db, const char *username, const char *password, UserRole *out_role)
{
    if (!db || !username || !password || !out_role)
    {
        log_event(LOG_WARN, "Authentication failed: invalid pointers provided.");
        return USER_ERR_INVALID_PARAM;
    }

    char input_hash[SHA256_HEX_SIZE] = {0};
    if (user_hash_password(password, input_hash, sizeof(input_hash)) != 0)
    {
        log_event(LOG_ERROR, "Hashing failed for provided password.");
        return USER_ERR_INTERNAL;
    }

    const char *sql = "SELECT password_hash, role FROM users WHERE username = ?;";
    sqlite3_stmt *stmt = NULL;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        log_event(LOG_ERROR, "Auth query preparation failed: %s", sqlite3_errmsg(db));
        return USER_ERR_DB;
    }

    sqlite3_bind_text(stmt, 1, username, strlen(username), SQLITE_STATIC);

    UserStatus status = USER_ERR_UNAUTHORIZED;

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *stored_hash = (const char *)sqlite3_column_text(stmt, 0);
        const char *role_str = (const char *)sqlite3_column_text(stmt, 1);

        if (stored_hash && strcmp(input_hash, stored_hash) == 0)
        {
            *out_role = user_string_to_role(role_str);
            if (*out_role != USER_ROLE_INVALID)
            {
                status = USER_SUCCESS;
                log_event(LOG_AUDIT, "User '%s' authenticated successfully (Role: %s).", username, role_str);
            }
            else
            {
                status = USER_ERR_DB;
                log_event(LOG_ERROR, "Invalid role '%s' found for user '%s' in DB.", role_str, username);
            }
        }
    }
    else
    {
        status = USER_ERR_NOT_FOUND;
        log_event(LOG_WARN, "Authentication failed: user '%s' not found.", username);
    }

    sqlite3_finalize(stmt);
    return status;
}

int user_hash_password(const char *password, char *out_hash_hex, size_t out_size)
{
    if (!password || !out_hash_hex || out_size < SHA256_HEX_SIZE)
        return -1;

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx)
        return -1;

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1 ||
        EVP_DigestUpdate(ctx, password, strlen(password)) != 1 ||
        EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1)
    {
        EVP_MD_CTX_free(ctx);
        return -1;
    }

    EVP_MD_CTX_free(ctx);

    for (unsigned int i = 0; i < hash_len; i++)
        snprintf(out_hash_hex + (i * 2), out_size - (i * 2), "%02x", hash[i]);

    return 0;
}