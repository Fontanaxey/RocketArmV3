#include <ncurses.h>
#include <stdio.h>
#include "ui_ncurses.h"
#include "protocol.h"
#include "errhandler.h"

int main()
{
    log_event(ACCESS, "memedesimo");
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

    ui_init();

    while (running)
    {
        ui_render(arm_items, num_items, selected);
        int ch = getch();

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
                arm_items[selected].current_value += 1;
            break;

        case KEY_LEFT:
            if (arm_items[selected].current_value > ANGLE_MIN)
                arm_items[selected].current_value -= 1;
            break;

        case 'q':
        case 'Q':
            running = 0;
            break;

        default:
            break;
        }
    }

    ui_cleanup();

    printf("\n[TEST MODE] Interfaccia chiusa correttamente.\n");
    printf("Lo stato finale del braccio era:\n");
    for (int i = 0; i < num_items; i++)
        printf(" - %-12s: %d\n", arm_items[i].name, arm_items[i].current_value);
    return 0;
}