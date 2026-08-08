#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ui_ncurses.h"
#include "protocol.h"
#include "err_handler.h"
#include "serial_linux.h"
#include "version.h"
#include "users_handler.h"

#define DB_PATH "data/shadow.db"
#define MAX_LOGIN_ATTEMPTS 3

static void read_password_masked(char *buf, size_t size)
{
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    if (fgets(buf, size, stdin))
    {
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n')
            buf[len - 1] = '\0';
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
}

int main()
{
    char start_msg[64] = {0};
    snprintf(start_msg, sizeof(start_msg), "%s v%s started", APP_NAME, APP_VERSION);
    log_event(LOG_INFO, "%s", start_msg);

    sqlite3 *db = NULL;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
    {
        log_event(LOG_ERROR, "cannot open %s: %s", DB_PATH, sqlite3_errmsg(db));
        fprintf(stderr, "Critical DB error. Unable to start application.\n");
        sqlite3_close(db);
        return EXIT_FAILURE;
    }

    UserRole user_role = USER_ROLE_INVALID;
    char username[64] = {0};
    char password[64] = {0};
    int authenticated = 0;

    struct stat st = {0};
    if (stat("data", &st) == -1)
        if (mkdir("data", 0700) != 0)
        {
            log_event(LOG_FATAL, "impossibile creare la directory 'data'");
            return EXIT_FAILURE;
        }

    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
    {
        log_event(LOG_ERROR, "impossibile aprire %s: %s", DB_PATH, sqlite3_errmsg(db));
        return EXIT_FAILURE;
    }

    const char *schema_sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    username TEXT UNIQUE NOT NULL,"
        "    password_hash TEXT NOT NULL,"
        "    role TEXT NOT NULL CHECK(role IN ('admin', 'operator', 'viewer')),"
        "    failed_attempts INTEGER NOT NULL DEFAULT 0,"
        "    locked_until DATETIME DEFAULT NULL,"
        "    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "    last_login DATETIME DEFAULT NULL"
        ");";

    char *err_msg = NULL;
    if (sqlite3_exec(db, schema_sql, NULL, NULL, &err_msg) != SQLITE_OK)
    {
        log_event(LOG_FATAL, "Inizializzazione schema DB fallita: %s", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return EXIT_FAILURE;
    }

    printf("=== AUTHENTICATION REQUIRED ===\n");
    for (int attempt = 1; attempt <= MAX_LOGIN_ATTEMPTS; attempt++)
    {
        printf("Username: ");
        if (!fgets(username, sizeof(username), stdin))
            break;
        username[strcspn(username, "\n")] = '\0';

        printf("Password: ");
        read_password_masked(password, sizeof(password));

        UserStatus status = user_authenticate(db, username, password, &user_role);

        if (status == USER_SUCCESS)
        {
            authenticated = 1;
            log_event(LOG_INFO, "Authentication successful for '%s' (Role: %s)",
                      username, user_role_to_string(user_role));
            break;
        }

        log_event(LOG_WARN, "Login attempt failed for '%s' (Attempt %d/%d)",
                  username, attempt, MAX_LOGIN_ATTEMPTS);
        printf("invalid credentials\n\n");
    }

    sqlite3_close(db);

    if (!authenticated)
        return EXIT_FAILURE;

    if (user_role == USER_ROLE_VIEWER)
        log_event(LOG_INFO, "User '%s' has view-only permissions (VIEWER).", username);

    MenuItem arm_items[] = {
        {"Base Motor", 0x01, DEFAULT_POS},
        {"Joint 1", 0x02, DEFAULT_POS},
        {"Joint 2", 0x03, DEFAULT_POS},
        {"Joint 3", 0x04, DEFAULT_POS},
        {"Joint 4", 0x05, DEFAULT_POS},
        {"Gripper 1", 0x06, 0}};

    int num_items = sizeof(arm_items) / sizeof(MenuItem);
    int selected = 0;
    int running = 1;

    int serial_fd = serial_init("/tmp/ttyV0");
    int is_connected = (serial_fd >= 0);

    if (!is_connected)
        log_event(LOG_WARN, "Running in SIMULATION MODE (No hardware detected)");

    ui_init();

    while (running)
    {
        ui_render(arm_items, num_items, selected, is_connected);

        int ch = getch();
        int value_changed = 0;

        switch (ch)
        {
        case KEY_UP:
            selected = (selected - 1 + num_items) % num_items;
            break;

        case KEY_DOWN:
            selected = (selected + 1) % num_items;
            break;

        case KEY_RIGHT:
            if (arm_items[selected].current_value < ANGLE_MAX)
            {
                arm_items[selected].current_value += 1;
                value_changed = 1;
            }
            break;

        case KEY_LEFT:
            if (arm_items[selected].current_value > ANGLE_MIN)
            {
                arm_items[selected].current_value -= 1;
                value_changed = 1;
            }
            break;

        case 'q':
        case 'Q':
            running = 0;
            break;
        }

        if (value_changed)
        {
            if (serial_fd < 0)
                is_connected = 0;
            else
            {
                RobotPacket pkt;
                protocol_create_packet(&pkt, arm_items[selected].command_id, (uint8_t)arm_items[selected].current_value);
                if (serial_send(serial_fd, pkt) != 0)
                {
                    if (is_connected)
                        log_event(LOG_WARN, "Communication lost: serial_send failed");
                    is_connected = 0;
                }
                else
                    is_connected = 1;
            }
        }
    }
    ui_cleanup();
    if (serial_fd >= 0)
        serial_close(serial_fd);
    printf("\n[FINAL STATE] Interface closed correctly.\n");
    for (int i = 0; i < num_items; i++)
        printf(" - %-12s: %d\n", arm_items[i].name, arm_items[i].current_value);
    return 0;
}