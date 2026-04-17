#include <ncurses.h>
#include "ui_ncurses.h"

void ui_init(void)
{
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);
    start_color();
}

void ui_render(MenuItem *items, int count, int selected_idx)
{
    clear();

    attron(A_BOLD | A_UNDERLINE);
    mvprintw(0, 0, "--- RocketArm V3 Controller ---");
    attroff(A_BOLD | A_UNDERLINE);

    mvprintw(2, 0, "Use UP/DOWN to select, LEFT/RIGHT to move, Q to quit");

    for (int i = 0; i < count; i++)
        if (i == selected_idx)
        {
            attron(A_REVERSE);
            mvprintw(4 + i, 2, "> %-12s : [%3d]", items[i].name, items[i].current_value);
            attroff(A_REVERSE);
        }
        else
            mvprintw(4 + i, 2, "  %-12s : [%3d]", items[i].name, items[i].current_value);

    refresh();
}

void ui_cleanup(void)
{
    endwin();
}