
#ifndef LOGIN_PUZZLE_H
#define LOGIN_PUZZLE_H

#include "display_setup.h"




void init_rect(void);
void draw_placeholders(void);
void draw_showbutton(int counter);
void draw_reset_button(void);
bool login_game(tsPoint_t touch_raw);
int what_was_touched(int x, int y);
int get_section(int x, int y);

bool is_section_with_letter(int section);
bool random_letter_generation(bool init);
char get_letter(int section);
void remove_section_letters(void);
void blink_section_letters(bool blink);
void draw_placeholder_letters(void);
void shift_letter(int cur_section, int cur_cursor_section);
bool isNewRecToCome(int rect_selected_num);
bool is_rect_here(int section);
int find_next_free_placeholder(bool right, int rec_section);
bool is_rect_puzzle_solved(void);
void dissapearing_letters(void);

#endif