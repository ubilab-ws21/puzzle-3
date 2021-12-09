#ifndef DISPLAY_STUFF_H
#define DISPLAY_STUFF_H

void dissapearing_letters(void);
void sliding_bars(int encoder_num);
int convert_encoder2display_x(int encoder_value);
int convert_encoder2display_y(int x_val, int max_y_val);

#endif
