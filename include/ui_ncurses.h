#ifndef UI_NCURSES_H
#define UI_NCURSES_H

#include "protocol.h"

typedef struct
{
    char *name;
    uint8_t command_id;
    int current_value;
} MenuItem;

void ui_init(void);
void ui_cleanup(void);

void ui_render(MenuItem *items, int count, int selected_idx);

#endif