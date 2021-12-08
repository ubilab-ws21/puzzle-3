#include "display_stuff.h"
#include "display_setup.h"


#include "Fonts/FreeSerifBoldItalic9pt7b.h"

#define LETTER_OFFSET 25
tsPoint_t letter;
tsPoint_t calibrated;
tsPoint_t midpoint;

static int initialized_game =0;

void dissapearing_letters()
{
    static tsMatrix_t local_matrix = gettsMatrix();
    static Adafruit_RA8875 local_tft = gettft();

    if(!initialized_game)
    {
        Serial.println("Initializing game");
        local_tft.setFont(&FreeSerifBoldItalic9pt7b);

        letter.x = 400;
        letter.y = 240;
        calibrateTSPoint(&calibrated, &letter, &local_matrix );
        Serial.println(calibrated.x);
        Serial.println(calibrated.y);
        char mychar = 'A';
        local_tft.drawChar(letter.x,letter.y, mychar, RA8875_BLUE, RA8875_WHITE, 7);

        midpoint.x = letter.x + LETTER_OFFSET;
        midpoint.y = letter.y + LETTER_OFFSET;
        initialized_game = 1;
    }    

    tsPoint_t raw;
    tsPoint_t calibrated;

    // Wait around for a touch event 
    waitForTouchEvent(&raw);

    //Calcuate the real X/Y position based on the calibration matrix 
    calibrateTSPoint(&calibrated, &raw, &local_matrix );

    Serial.println("Calibrated letter: ");
    Serial.println(letter.x);
    Serial.println(letter.y);    

    if(abs(calibrated.x - midpoint.x) < 40 && abs(calibrated.y - midpoint.y) < 40)
    {
        Serial.println("Hit button");
        local_tft.drawChar(letter.x,letter.y, 'A', RA8875_WHITE, RA8875_WHITE, 7);
    }
    else
    { 
        Serial.println("Failed: ");
        //local_tft.drawChar(letter.x,letter.y, 'A', RA8875_YELLOW, RA8875_WHITE, 7);
    }
    Serial.print("touchscreen sensed at: ");
    Serial.println(calibrated.x);
    Serial.println(calibrated.y);
    // Draw a single pixel at the calibrated point 
    local_tft.fillCircle(calibrated.x, calibrated.y, 3, RA8875_BLACK);
    
}