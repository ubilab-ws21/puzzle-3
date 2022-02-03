#include "display_stuff.h"


unsigned long last_touch_time;
int frequency;

int last_action = last_action_none;

void set_last_action(int last)
{
    last_action = last;
}

int get_last_action()
{
    return last_action;
}

void set_solved_frequency(int freq){
    frequency = freq;
}

int get_solved_frequency(){
    return frequency;
}


/**************************************************************************/
/*!
    @brief  Waits for a touch or encoder event
*/
/**************************************************************************/
int handleTouchEvent(tsPoint_t * point)
{
    static Adafruit_RA8875 local_tft = gettft();

    /* Clear the touch data object and placeholder variables */
    memset(point, 0, sizeof(tsPoint_t));

    /* Clear any previous interrupts to avoid false buffered reads */
    uint16_t x, y;
    local_tft.touchRead(&x, &y);
    delay(1);

    /* Make sure this is really a touch event */
    if (local_tft.touched() && millis() - last_touch_time > 300)
    {
    local_tft.touchRead(&x, &y);
    point->x = x;
    point->y = y;
    //Serial.print("Touch: ");
    //Serial.print(point->x); Serial.print(", "); Serial.println(point->y);

    last_touch_time = millis();
    last_action = last_action_touch;
    }
    else
    {
    point->x = 0;
    point->y = 0;
    }
    return 0;
}


/**************************************************************************/
/*!
    @brief  Waits for a touch or encoder event
*/
/**************************************************************************/
/*
int waitForTouchorEncoderEvent(tsPoint_t * point)
{

    static Adafruit_RA8875 local_tft = gettft();

    //Clear the touch data object and placeholder variables 
    memset(point, 0, sizeof(tsPoint_t));

    // Clear any previous interrupts to avoid false buffered reads
    uint16_t x, y;
    local_tft.touchRead(&x, &y);
    delay(1);

    // Wait around for a new touch event (INT pin goes low) 
    while (digitalRead(RA8875_INT))
    {
        int enc_val = check_game_encoders();
        switch(enc_val)
        {
            case 1:
                last_action = last_action_encoder1;
                return enc_val;
                break;
            case 2:
                last_action = last_action_encoder2;
                return enc_val;
                break;
            case 3:
                last_action = last_action_encoder3;
                return enc_val;
                break;
            default:
                last_action = last_action_none;
                break;
        }
        if(vol_encoder_triggered())
            return 4;
    }

    // Make sure this is really a touch event
    if (local_tft.touched() && millis() - last_touch_time > 300)
    {
    local_tft.touchRead(&x, &y);
    point->x = x;
    point->y = y;
    Serial.print("Touch: ");
    Serial.print(point->x); Serial.print(", "); Serial.println(point->y);

    last_touch_time = millis();
    last_action = last_action_touch;
    }
    else
    {
    point->x = 0;
    point->y = 0;
    }
    return 0;
}*/


void section_to_xy(int section, int* x, int* y)
{
    int section_y = section / NUM_SECTIONS_X;
    *y = section_y * REC_SIZE_Y + REC_MIN_Y;

    *x = (section - (section_y * NUM_SECTIONS_X)) * REC_SIZE_X;
}


void first_screen()
{
    static Adafruit_RA8875 local_tft =  gettft();

    //local_tft.sleep(true);
    //local_tft.sleep(false);
    local_tft.graphicsMode();
    fill_display(RA8875_BLACK);
 
    uint16_t lcd_buffer[1000];
    for(int i = 0; i<1000; i++)
    {
        lcd_buffer[i] = RA8875_WHITE;
    }

    for(int i = 0; i<480; i+=4)
    {
        local_tft.drawPixels(lcd_buffer, 800, 300, i);
    }
}



void final_screen()
{
    static Adafruit_RA8875 local_tft = gettft();
    local_tft.fillScreen(RA8875_BLUE);

    local_tft.textMode();

    local_tft.textSetCursor(140, 100);

    local_tft.textTransparent(RA8875_RED);
    local_tft.textEnlarge(2);
    local_tft.textWrite("Code: ");

    local_tft.graphicsMode();

    char password[5] = "1234";
    int x, y;
    
    for(int i = 0; i<4; i++)
    {
        section_to_xy(i+13, &x, &y);
        local_tft.drawChar(x+30, y+30, password[i], RA8875_RED, RA8875_BLUE, 10);
        //placeholder_letters[i] = {};
    }
}