#include <ncurses.h>
#include <stdlib.h>
#include "ui_ncurses.h"
#include "errhandler.h"

void ui_init(void)
{
    if (initscr() == NULL)
    {
        log_event(FATAL, "Cannot initialize ncurses (initscr failed)");
        exit(EXIT_FAILURE);
    }

    raw();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    log_event(ACCESS, "UI module initialized successfully");
}

void ui_render(MenuItem *items, int count, int selected_idx)
{
    erase();

    attron(A_BOLD | A_UNDERLINE);
    mvprintw(0, 0, "--- RocketArm V3 Controller ---");
    attroff(A_BOLD | A_UNDERLINE);

    mvprintw(2, 0, "Use UP/DOWN to select, LEFT/RIGHT to move, Q to quit");

    int max_items = (LINES - 4 > 0) ? (LINES - 4) : 0;
    int display_count = (count < max_items) ? count : max_items;

    for (int i = 0; i < display_count; i++)
    {
        if (i == selected_idx)
        {
            attron(A_REVERSE);
            mvprintw(4 + i, 2, "> %-12s : [%3d]", items[i].name, items[i].current_value);
            attroff(A_REVERSE);
        }
        else
        {
            mvprintw(4 + i, 2, "  %-12s : [%3d]", items[i].name, items[i].current_value);
        }
    }

    refresh();
}

void ui_cleanup(void)
{
    log_event(ACCESS, "UI module shutting down");
    endwin();
}