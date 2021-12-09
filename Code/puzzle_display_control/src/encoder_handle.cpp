#include "encoder_handle.h"


#include <ESP32Encoder.h>

class Encoders {
    public:
        ESP32Encoder _encoder;
        int encoder_value;
        int old_value;
};
/*
ESP32Encoder encoder1;
ESP32Encoder encoder2;
ESP32Encoder encoder3;

struc
*/

Encoders encoder1;
Encoders encoder2;
Encoders encoder3;

void init_encoder()
{

    ESP32Encoder::useInternalWeakPullResistors=UP;

	// use pin 19 and 18 for the first encoder
	encoder1._encoder.attachHalfQuad(32, 33);
	// use pin 17 and 16 for the second encoder
	//encoder2.attachHalfQuad(17, 16);
		
	// set starting count value after attaching
	encoder1._encoder.setCount(0);

	// clear the encoder's raw count and set the tracked count to zero
	//encoder2.clearCount();
    encoder1.encoder_value = 0;
    encoder1.old_value = 0;
	Serial.println("Encoder Start = " + String((int32_t)encoder1._encoder.getCount()));

    // set starting count value after attaching
	encoder2._encoder.setCount(0);

	// clear the encoder's raw count and set the tracked count to zero
	//encoder2.clearCount();
    encoder2.encoder_value = 0;
    encoder2.old_value = 0;
}

int old_val = 0;
int check_encoder()
{
    if (encoder1.encoder_value !=  encoder1._encoder.getCount())
    {
        if(encoder1._encoder.getCount() > -12 && encoder1._encoder.getCount() < 12)
        {
            encoder1.encoder_value = encoder1._encoder.getCount();
            Serial.println("Encoder count 1 = " + String((int32_t)encoder1.encoder_value));
            return 1;
        }
        else
            encoder1._encoder.setCount(encoder1.encoder_value);
    }
/*
    if (encoder2.old_value != new_encoder_value)
    {
        if(new_encoder_value > -12 && new_encoder_value < 12)
        {
            encoder2.encoder_value = new_encoder_value;
            encoder2.old_value = encoder2.encoder_value;
            Serial.println("Encoder count 2 = " + String((int32_t)encoder2.encoder_value));
            return 2;
        }
        else
            encoder1._encoder.setCount(old_val);
    }*/
    return 0;
}

int encoder_get_value(int encoder_num)
{
    if(encoder_num == 1)
    {
        return encoder1.encoder_value;
    }
    if(encoder_num ==2)
    {
        return encoder2.encoder_value;
    }
    if(encoder_num ==3)
    {
        return encoder3.encoder_value;
    }
    return -1;
}