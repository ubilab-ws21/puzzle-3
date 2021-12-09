#include "display_stuff.h"
#include "display_setup.h"
#include "encoder_handle.h"


#include "Fonts/FreeSerifBoldItalic9pt7b.h"

#define LETTER_OFFSET 10

#define MAX_X_VAL 700
#define MIN_X_VAL 100
#define MEAN_X_VAL 400

#define MAX_Y_VAL_1 470
#define MIN_Y_VAL_1 420

#define MAX_Y_VAL_2 400
#define MIN_Y_VAL_2 350

#define MAX_Y_VAL_3 340
#define MIN_Y_VAL_3 290

#define STEP_SIZE 25

#define X_Y_RATIO double(MAX_Y_VAL_1-MIN_Y_VAL_1)/double(MAX_X_VAL-MIN_X_VAL)


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
        local_tft.drawChar(letter.x,letter.y, mychar, RA8875_BLUE, BACKGROUND_COLOR, 5);

        midpoint.x = letter.x + LETTER_OFFSET;
        midpoint.y = letter.y + LETTER_OFFSET;
        initialized_game = 1;
    }    

    tsPoint_t raw;
    tsPoint_t calibrated;

    int hit = 0;
    while(hit == 0)
    {
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
            local_tft.drawChar(letter.x,letter.y, 'A', RA8875_WHITE, RA8875_WHITE, 5);
            initialized_game = 0;
            hit = 1;
        }
        else
        { 
            Serial.println("Failed: ");
            local_tft.drawChar(letter.x,letter.y, 'A', RA8875_YELLOW, RA8875_WHITE,5);
        }
        Serial.print("touchscreen sensed at: ");
        Serial.println(calibrated.x);
        Serial.println(calibrated.y);
        // Draw a single pixel at the calibrated point 
        local_tft.fillCircle(calibrated.x, calibrated.y, 3, RA8875_BLACK);
    }
}

void sliding_bars(int encoder_num)
{
    static Adafruit_RA8875 local_tft = gettft();
    int32_t color;
    int max_Y_val = 0;
    int min_Y_val = 0;
    if(encoder_num == 1)
    {
        color = RA8875_CYAN;
        max_Y_val = MAX_Y_VAL_1;
        min_Y_val = MIN_Y_VAL_1;
    }
    else if(encoder_num == 2)
    {
        color = RA8875_GREEN;
        max_Y_val = MAX_Y_VAL_2;
        min_Y_val = MIN_Y_VAL_2;
    }
    else if(encoder_num == 3)
    {
        color = RA8875_MAGENTA;
        max_Y_val = MAX_Y_VAL_3;
        min_Y_val = MIN_Y_VAL_3;
    }

    local_tft.fillTriangle(MIN_X_VAL, max_Y_val, MAX_X_VAL, max_Y_val, MAX_X_VAL, min_Y_val, BACKGROUND_COLOR);
    local_tft.drawTriangle(MIN_X_VAL, max_Y_val, MAX_X_VAL, max_Y_val, MAX_X_VAL, min_Y_val, color);

    int encoder_value = encoder_get_value(encoder_num);

    int bar_fill_value_x = convert_encoder2display_x(encoder_value);
    int bar_fill_value_y = convert_encoder2display_y(bar_fill_value_x, max_Y_val);

    Serial.println("encoder value");
    Serial.print(encoder_value);

    local_tft.fillTriangle(MIN_X_VAL, max_Y_val, bar_fill_value_x, max_Y_val, bar_fill_value_x, bar_fill_value_y, color);
}

int convert_encoder2display_x(int encoder_value)
{
    int return_val = MEAN_X_VAL;

    return_val += 25 * encoder_value;

    return return_val;
}

int convert_encoder2display_y(int x_val, int max_y_val)
{
    int x_length = x_val-MIN_X_VAL;

    int return_val  = max_y_val - X_Y_RATIO * x_length;

    return return_val;
}