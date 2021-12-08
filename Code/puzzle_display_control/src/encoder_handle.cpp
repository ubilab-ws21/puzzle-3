#include "encoder_handle.h"


#include <ESP32Encoder.h>

ESP32Encoder encoder;

void init_encoder()


{

    ESP32Encoder::useInternalWeakPullResistors=UP;

	// use pin 19 and 18 for the first encoder
	encoder.attachHalfQuad(32, 33);
	// use pin 17 and 16 for the second encoder
	//encoder2.attachHalfQuad(17, 16);
		
	// set starting count value after attaching
	encoder.setCount(37);

	// clear the encoder's raw count and set the tracked count to zero
	//encoder2.clearCount();
	Serial.println("Encoder Start = " + String((int32_t)encoder.getCount()));

}

int old_val = 0;
int check_encoder()
{
    int encoder_value = encoder.getCount();
    if (old_val != encoder_value)
    {
        old_val = encoder_value;
        Serial.println("Encoder count = " + String((int32_t)encoder_value));
        return 1;
    }
    return 0;
    
}