#ifndef ENCODER_HANDLE_H
#define ENCODER_HANDLE_H

#define NUM_ENCODERS_DEFINED 2

void init_encoder(void);
int check_encoder(void);

int encoder_get_value(int encoder_value);
void encoder_set_value(int encoder_num, int value);

#endif