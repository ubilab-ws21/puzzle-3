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
void rectangles_game(tsPoint_t touch_raw);
int get_section(int x, int y);
bool is_section_with_letter(int section);
char get_letter(int section);
bool isNewRecToCome(int rect_selected_num);
rect select_rectangle(int num);
void save_rect_data(int rect_num, tsPoint_t point, int color);
int position_occupied(int section);
void dissapearing_letters(void);
void sliding_bars(int encoder_num);
int convert_encoder2display_x(int encoder_value);
int convert_encoder2display_y(int x_val, int max_y_val);
int waitForTouchorEncoderEvent(tsPoint_t *point);

#endif
