#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include "ui_ncurses.h"
#include "protocol.h"
#include "errhandler.h"
#include "serial_linux.h"
#include "version.h"

int main()
{
    char start_msg[64];
    snprintf(start_msg, sizeof(start_msg), "%s v%s started", APP_NAME, APP_VERSION);
    log_event(ACCESS, start_msg);

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
        log_event(WARNING, "Running in SIMULATION MODE (No hardware detected)");

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
                        log_event(WARNING, "Communication lost: serial_send failed");
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