#include "encoder_handle.h"
#include <ESP32Encoder.h>

class Encoders {
    public:
        ESP32Encoder _encoder;
        int encoder_value;
        int old_value;
};

Encoders encoder[NUM_ENCODERS_DEFINED];

Encoders encoder_vol;

Encoders encoder_ant;

void init_encoder()
{

    ESP32Encoder::useInternalWeakPullResistors=UP;

    

	// use pin 19 and 18 for the first encoder
	encoder[0]._encoder.attachSingleEdge(33, 32);
	// use pin 17 and 16 for the second encoder
	
    if(NUM_ENCODERS_DEFINED > 1)
    {
        encoder[1]._encoder.attachSingleEdge(26, 25);
    }
    if(NUM_ENCODERS_DEFINED > 2)
    {
        encoder[2]._encoder.attachSingleEdge(17, 16);
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
    /*
    encoder[0]._encoder.setFilter(1000);
    encoder[1]._encoder.setFilter(1000);
    encoder[2]._encoder.setFilter(1000);
*/

    // init volume encoder
    encoder_ant._encoder.attachSingleEdge(34, 35);

    // set starting count value after attaching
    encoder_ant._encoder.setCount(0);

    // clear the encoder's raw count and set the tracked count to zero
    //encoder2.clearCount();
    encoder_ant.encoder_value = 0;
    encoder_ant.old_value = 0;



    // init volume encoder
    encoder_vol._encoder.attachSingleEdge(22, 21);

    // set starting count value after attaching
    encoder_vol._encoder.setCount(15);

    // clear the encoder's raw count and set the tracked count to zero
    //encoder2.clearCount();
    encoder_vol.encoder_value = 0;
    encoder_vol.old_value = 0;
}

int old_val = 0;

/***********************************************************************
* This function checks if any of the game encoders (the encoders on the top)
* have been triggered and if yes, it saves the new value and 
* returns the corresponding encoder number 
***********************************************************************/
int check_game_encoders()
{
    int i;
    for(i = 0; i<NUM_ENCODERS_DEFINED; i++)
    {
        if (encoder[i].encoder_value !=  encoder[i]._encoder.getCount())
        {
            if(encoder[i]._encoder.getCount() >= -12 && encoder[i]._encoder.getCount() <= 12)
            {
                encoder[i].encoder_value = encoder[i]._encoder.getCount();
                Serial.println("Encoder no " + String(i) + ", count  = " + String((int32_t)encoder[i].encoder_value));
            }
            else
            {
                encoder[i]._encoder.setCount(encoder[i].encoder_value);
            }
            return (i+1);       
        }
    }
    return 0;
}

/***********************************************************************
* This function checks if the volume encoder (the encoder on the side)
* has been triggered and if yes, it saves the new value, sets the ne_volume 
* variable accordingly and returns true
***********************************************************************/
bool check_vol_encoder(int* new_volume)
{
    if (encoder_vol.encoder_value !=  encoder_vol._encoder.getCount())
        {
            if(encoder_vol._encoder.getCount() >= 0 && encoder_vol._encoder.getCount() < 26)
            {
                encoder_vol.encoder_value = encoder_vol._encoder.getCount();
                Serial.println("Encoder vol count  = " + String((int32_t)encoder_vol.encoder_value));
                *new_volume = encoder_vol.encoder_value;
            }
            else
            {
                encoder_vol._encoder.setCount(encoder_vol.encoder_value);
            }
            return true;       
        }
    return false;
}

/***********************************************************************
* This function checks if the antenna encoder (the encoder inside the radio)
* has been triggered and if yes, it makes sure that the value is always between 
* -12 and 12, it saves the new value and returns an unsigned 
* version of the value, which is between 0 and 12.
***********************************************************************/
unsigned int check_ant_encoder()
{
    int ant_value;
    if (encoder_ant.encoder_value !=  encoder_ant._encoder.getCount())
        {
            if(encoder_ant._encoder.getCount() > -12 && encoder_ant._encoder.getCount() <= 12)
            {
                encoder_ant.encoder_value = encoder_ant._encoder.getCount();
                
                Serial.println("Encoder ant count  = " + String((int32_t)encoder_ant.encoder_value));
            }
            else if(encoder_ant._encoder.getCount() <= -12)
            {
                encoder_ant._encoder.setCount(abs(encoder_ant._encoder.getCount())-2);
                encoder_ant.encoder_value = encoder_ant._encoder.getCount();
            }        
            else if(encoder_ant._encoder.getCount() > 12)
            {
                encoder_ant._encoder.setCount(-(encoder_ant._encoder.getCount())+2);
                encoder_ant.encoder_value = encoder_ant._encoder.getCount();
            }
        }

    ant_value = abs(encoder_ant.encoder_value);

    return ant_value;
}

/***********************************************************************
* returns the saved value of the corresponding encoder
***********************************************************************/
int encoder_get_value(int encoder_num)
{
    if(encoder_num <= NUM_ENCODERS_DEFINED && encoder_num > 0)
    {
        return encoder[encoder_num-1].encoder_value;
    }   
    return -1;
}

/***********************************************************************
* sets the saved value of the corresponding encoder
***********************************************************************/
void encoder_set_value(int encoder_num, int value)
{
    Serial.print("set encoder ");
    Serial.print(encoder_num);
    Serial.print(" to value ");
    if(encoder_num <= NUM_ENCODERS_DEFINED && encoder_num > 0)
    {
        Serial.print(value);
        encoder[encoder_num-1].encoder_value = value; 
        encoder[encoder_num-1]._encoder.setCount(encoder[encoder_num-1].encoder_value);
    }
    else if(encoder_num == 4)
    {
        // encoder 4 is the antenna controller
        encoder_ant.encoder_value = value;
        encoder_ant._encoder.setCount(encoder_ant.encoder_value);
    }
    else if(encoder_num == 5)
    {
        // encoder 5 is the volume controller
        encoder_vol.encoder_value = value;
        encoder_vol._encoder.setCount(encoder_vol.encoder_value);
    }
}