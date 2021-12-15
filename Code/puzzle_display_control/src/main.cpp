#include "display_setup.h"
#include "display_stuff.h"
#include "encoder_handle.h"

#include <stdio.h>
#include <stdlib.h>

#define LETTER_GAME 1

//#include "Fonts/FreeSerfBoldItalic9pt7b.h"

/**************************************************************************/
/*!
*/
/**************************************************************************/

tsPoint_t cal_letter;

void setup()
{
    Serial.begin(9600);
    Serial.println("Hello, RA8875!");

    init_display();

  //tft.setFont(&FreeSerifBoldItalic9pt7b);
    init_encoder();

#ifndef LETTER_GAME
    sliding_bars(1);
    sliding_bars(2);
    sliding_bars(3);
#else
    init_rect();
#endif
}

/**************************************************************************/
/*!
*/
/**************************************************************************/
int encoder_triggered = 0;
void loop()
{
tsPoint_t raw;

#ifdef LETTER_GAME
    //dissapearing_letters();
    
    while(1)
    {
        waitForTouchorEncoderEvent(&raw);
        rectangles_game(raw);   
    }
#else
    while(1)
    {
        encoder_triggered = check_encoder();
        if (encoder_triggered)
        {
            break;
        }
    }
    sliding_bars(encoder_triggered);
#endif
    

}