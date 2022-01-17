#ifndef DISPLAY_STUFF_H
#define DISPLAY_STUFF_H

#include "display_setup.h"

struct rect{
    tsPoint_t tr_corner;
    tsPoint_t tr_corner_cal;
    int color;
    int section; 
};

void init_rect(void);
void draw_placeholders(void);
void draw_showbutton(int counter);
bool rectangles_game(tsPoint_t touch_raw);
int get_section(int x, int y);
void section_to_xy(int section, int* x, int* y);
bool is_section_with_letter(int section);
bool random_letter_generation(bool init);
char get_letter(int section);
void draw_placeholder_letters(void);
void shift_letter(int cur_section);
bool isNewRecToCome(int rect_selected_num);
int who_is_here(int section);
int position_occupied(int section);
bool is_rect_puzzle_solved(void);
void dissapearing_letters(void);
bool sliding_bars(int encoder_num);
void draw_numbers(const char* string, int cursor_x, int cursor_y);
int convert_encoder2display_x(int encoder_value);
int convert_encoder2display_y(int x_val, int max_y_val);
int waitForTouchorEncoderEvent(tsPoint_t *point);
void first_screen(void);
void final_screen(void);

#endif
