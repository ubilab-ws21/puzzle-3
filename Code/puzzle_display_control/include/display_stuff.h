#ifndef DISPLAY_STUFF_H
#define DISPLAY_STUFF_H

#include "display_setup.h"

#define REC_MIN_Y 80
#define REC_SIZE_X 133
#define REC_SIZE_Y 100

#define NUM_SECTIONS_X 6
#define NUM_SECTIONS_Y 4


struct rect{
    tsPoint_t tr_corner;
    tsPoint_t tr_corner_cal;
    int color;
    int section; 
};

enum
{
    last_action_none = 0,
    last_action_encoder1 = 1,
    last_action_encoder2,
    last_action_encoder3,
    last_action_touch
};

int handleTouchEvent(tsPoint_t * point);
int waitForTouchorEncoderEvent(tsPoint_t *point);
void section_to_xy(int section, int* x, int* y);
void first_screen(void);
void final_screen(void);
void set_last_action(int last);
int get_last_action(void);
void set_solved_frequency(int freq);
int get_solved_frequency();

#endif
