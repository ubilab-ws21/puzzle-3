#ifndef FREQUENCY_PUZZLE_H
#define FREQUENCY_PUZZLE_H

#include "display_setup.h"

void init_sliding_bars(void);
bool sliding_bars(int encoder_num, tsPoint_t touch_raw, int init);
bool select_btn_pressed(tsPoint_t touch_raw);
void update_top_half(void);
int calc_cur_frequency(void);
void draw_numbers(const char* string, int cursor_x, int cursor_y);
int convert_encoder2display_x(int encoder_value);
int convert_encoder2display_y(int x_val, int max_y_val);


#endif