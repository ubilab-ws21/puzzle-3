#include "display_setup.h"
#include "display_stuff.h"
#include "encoder_handle.h"

#include <stdio.h>
#include <stdlib.h>

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
  
}

/**************************************************************************/
/*!
*/
/**************************************************************************/
void loop()
{

    dissapearing_letters();
    while(1)
    {
        if (check_encoder())
        {
            break;
        }
    }
    

}