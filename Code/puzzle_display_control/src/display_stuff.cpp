#include "display_stuff.h"
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <string.h>
//#include "time.h"

using namespace std;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedTime;
String hourStamp;
String minuteStamp;
String secondStamp;
String timeStamp;

//Local Time Variables
/*const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;*/

int hourNow, CurrentHour, UnitHour, TenHour;
int CurrentMinute, CurrentSecond;

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

struct tm timeinfo;

bool timeFail = true;
bool timeFailedBefore = true;

void init_time()
{
    // Initialize a NTPClient to get time
    timeClient.begin();
    // Set offset time in seconds to adjust for your timezone, for example:
    // GMT +1 = 3600
    // GMT +8 = 28800
    // GMT -1 = -3600
    // GMT 0 = 0
    timeClient.setTimeOffset(3600);
}


void setup_final_screen()
{

    int cur_time = millis();
    timeFail = true; 
    while(millis() - cur_time < 2000) {
        if(timeClient.update())
        {
            timeFail = false; 
            break;
        }
        timeClient.end();
        delay(100);
        timeClient.begin();
        timeClient.setTimeOffset(3600);
        delay(100);
    }

    if(timeFail == true)
    {
        Serial.println("failed to obtain time");
    }
    else
    {
        CurrentSecond = timeClient.getSeconds();
        CurrentMinute = timeClient.getMinutes();
        CurrentHour = timeClient.getHours();
        Serial.print("Current time: ");
        Serial.print(CurrentHour);
        Serial.print(" : ");
        Serial.print(CurrentMinute);
        Serial.print(" : ");
        Serial.print(CurrentSecond);    }

    delay(1000);
}


void final_screen()
{
    static Adafruit_RA8875 local_tft = gettft();

    //Serial.println(formattedTime);
    //Serial.println(CurrentSecond);

    if(timeFail == false)
    {
        CurrentSecond+=1;
        if(CurrentSecond==60)
        {
            CurrentSecond = 0;
            CurrentMinute+=1; 
            if(CurrentMinute == 60)
            {
                CurrentHour+=1;
                CurrentMinute = 0;
                if(CurrentHour == 24)
                {
                    CurrentHour = 0;
                }
            }
        }
        Serial.println(CurrentSecond);
    }
    else
    {
        int cur_time = millis();
        timeFail = true; 
        while(millis() - cur_time < 2000) {
            if(timeClient.update())
            {
                timeFail = false; 
                break;
            }
            timeClient.end();
            delay(100);
            timeClient.begin();
            timeClient.setTimeOffset(3600);
            delay(100);
        }

        if(timeFail == true)
        {
            Serial.println("failed to obtain time");
        }
        else
        {
            CurrentSecond = timeClient.getSeconds();
            CurrentMinute = timeClient.getMinutes();
            CurrentHour = timeClient.getHours();

            Serial.println(CurrentSecond);
        }
    }

    local_tft.textMode();

    if(timeFail == false)
    {
        local_tft.graphicsMode();
        local_tft.fillScreen(BACKGROUND_COLOR);
        local_tft.textMode();
        char unithourString[2];
        char tenhourString[2];
        itoa(CurrentHour%10, unithourString, 10); 
        itoa((CurrentHour/10)%10, tenhourString, 10); 

        char unitminuteString[2];
        char tenminuteString[2];
        itoa(CurrentMinute%10, unitminuteString, 10); 
        itoa((CurrentMinute/10)%10, tenminuteString, 10); 

        char unitsecondString[2];
        char tensecondString[2];
        itoa(CurrentSecond%10, unitsecondString, 10); 
        itoa((CurrentSecond/10)%10, tensecondString, 10); 

        local_tft.textSetCursor(270,200);
        local_tft.textEnlarge(3);
        local_tft.textTransparent(RA8875_RED);
        local_tft.textWrite(tenhourString);
        local_tft.textSetCursor(310,200);
        local_tft.textTransparent(RA8875_GREEN);
        local_tft.textWrite(tenhourString);
        local_tft.textSetCursor(350, 200);
        local_tft.textTransparent(RA8875_BLUE);
        local_tft.textWrite(":");
        local_tft.textSetCursor(380, 200);
        local_tft.textWrite(tenminuteString);
        local_tft.textSetCursor(420, 200);
        local_tft.textWrite(unitminuteString);
        local_tft.textSetCursor(460, 200);
        local_tft.textWrite(":");
        local_tft.textSetCursor(490, 200);
        local_tft.textWrite(tensecondString);
        local_tft.textSetCursor(530, 200);
        local_tft.textWrite(unitsecondString);
    }
    else
    {
        local_tft.textSetCursor(50,200);
        local_tft.textEnlarge(1);
        local_tft.textTransparent(RA8875_BLUE);
        local_tft.textWrite("Imagine the current time was displayed here...");
    }
    


    

    delay(1000);
}