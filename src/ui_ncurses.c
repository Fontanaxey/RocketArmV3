#include <ncurses.h>
#include <stdlib.h>
#include "ui_ncurses.h"
#include "err_handler.h"
#include "version.h"

void ui_init(void)
{
    if (initscr() == NULL)
    {
        log_event(LOG_FATAL, "Cannot initialize ncurses (initscr failed)");
        exit(EXIT_FAILURE);
    }

    raw();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    log_event(LOG_INFO, "UI module initialized successfully");
}

void ui_render(MenuItem *items, int num_items, int selected, int is_connected)
{
    erase();
    mvprintw(0, 2, "%s v%s", APP_NAME, APP_VERSION);
    move(1, 2);
    printw("--- ROCKET ARM V3 CONTROL --- (q to quit) APP_VERSION: %s", APP_VERSION);

    move(2, 2);
    if (is_connected)
    {
        attron(COLOR_PAIR(1));
        printw("[ ONLINE ]");
        attroff(COLOR_PAIR(1));
    }
    else
    {
        attron(COLOR_PAIR(2) | A_BLINK);
        printw("[! OFFLINE !]");
        attroff(COLOR_PAIR(2) | A_BLINK);
    }

    for (int i = 0; i < num_items; i++)
    {
        if (i == selected)
            attron(A_REVERSE);
        mvprintw(3 + i, 2, "%-15s: [%3d]", items[i].name, items[i].current_value);
        if (i == selected)
            attroff(A_REVERSE);
    }

    refresh();
}

void ui_cleanup(void)
{
    log_event(LOG_INFO, "UI module shutting down");
    endwin();
}