#include "encoder_handle.h"


#include <ESP32Encoder.h>

#define NUM_ENCODERS_DEFINED 1

class Encoders {
    public:
        ESP32Encoder _encoder;
        int encoder_value;
        int old_value;
};
/*
Encoders encoder1;
Encoders encoder2;
Encoders encoder3;*/

Encoders encoder[NUM_ENCODERS_DEFINED];

void init_encoder()
{

    ESP32Encoder::useInternalWeakPullResistors=UP;

	// use pin 19 and 18 for the first encoder
	encoder[0]._encoder.attachSingleEdge(32, 33);
	// use pin 17 and 16 for the second encoder
	
    if(NUM_ENCODERS_DEFINED > 1)
    {
        encoder[1]._encoder.attachSingleEdge(16, 17);
    }
    if(NUM_ENCODERS_DEFINED > 2)
    {
        encoder[2]._encoder.attachSingleEdge(25, 26);
    }
		
    int i;
    for(i = 0; i<NUM_ENCODERS_DEFINED; i++)
    {
        // set starting count value after attaching
        encoder[i]._encoder.setCount(0);

        // clear the encoder's raw count and set the tracked count to zero
        //encoder2.clearCount();
        encoder[i].encoder_value = 0;
        encoder[i].old_value = 0;
        Serial.println("Encoder Start = " + String((int32_t)encoder[0]._encoder.getCount()));

    }
}

int old_val = 0;
int check_encoder()
{
    int i;
    for(i = 0; i<NUM_ENCODERS_DEFINED; i++)
    {
        if (encoder[i].encoder_value !=  encoder[i]._encoder.getCount())
        {
        if(encoder[i]._encoder.getCount() > -12 && encoder[i]._encoder.getCount() < 12)
        {
            encoder[i].encoder_value = encoder[i]._encoder.getCount();
            Serial.println("Encoder count 1 = " + String((int32_t)encoder[i].encoder_value));
            return (i+1);
        }
        else
            encoder[i]._encoder.setCount(encoder[i].encoder_value);
        }
    }
    return 0;
}

int encoder_get_value(int encoder_num)
{
    if(encoder_num <= NUM_ENCODERS_DEFINED && encoder_num > 0)
        return encoder[encoder_num-1].encoder_value;
   
    return -1;
}

void encoder_set_value(int encoder_num, int value)
{
    if(encoder_num <= NUM_ENCODERS_DEFINED && encoder_num > 0)
    {
        encoder[encoder_num-1].encoder_value = value; 
        encoder[encoder_num-1]._encoder.setCount(encoder[encoder_num].encoder_value);
    }
}