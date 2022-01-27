#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include "display_setup.h"
#include "display_stuff.h"
#include "encoder_handle.h"

#include <stdio.h>
#include <stdlib.h>

// Audio interface
#define TX 4 
#define RX 15 

#define ESA 1       // folder number of Emergency Message
#define Ferdi 2     // folder number of Radio station message 
#define goodNews 3  // folder number of good News Radio
#define noise 10    // folder number of noise
#define rickroll 99 // default.
//#include "Fonts/FreeSerfBoldItalic9pt7b.h"

/**************************************************************************/
/*!
*/
/**************************************************************************/

tsPoint_t cal_letter;


SoftwareSerial DFPlayerSoftwareSerial(TX, RX);

// Name of Player
DFRobotDFPlayerMini mp3Player;

bool flagset = false; 
bool solved = false;
// the current state
int state;

void main_state_machine();

void setup()
{
    Serial.begin(9600);
    Serial.println("Hello, RA8875!");

    DFPlayerSoftwareSerial.begin(9600);


    mp3Player.begin(DFPlayerSoftwareSerial);
    Serial.println(mp3Player.readFileCounts());

    /* Equalizer: possible values
    *  mp3Player.EQ(DFPlayer_EQ_NORMAL);
    *  mp3Player.EQ(DFPLAYER_EQ_POP);
    *  mp3Player.EQ(DFPLAYER_EQ_ROCK);
    *  mp3Player.EQ(DFPLAYER_EQ_JAZZ);
    *  mp3Player.EQ(DFPLAYER_EQ_CLASSIC);
    *  mp3Player.EQ(DFPLAYER_EQ_BASS);
    */
    mp3Player.EQ(DFPLAYER_EQ_NORMAL);
    // Volume 1 to 30
    mp3Player.volume(15);

    init_display();
    init_encoder();

    state = 0;    
}

/**************************************************************************/
/*!
*/
/**************************************************************************/
int encoder_triggered = 0;
bool ant_correct = false;
int new_vol = 20;
tsPoint_t raw;
void loop()
{
    main_state_machine();
    
}


void main_state_machine()
{
    switch(state)
    {
    case 0:
        if(!flagset)
        {
            Serial.println("case 0");
            mp3Player.loopFolder(noise);
            first_screen();
            flagset = true;   
        }
        
        ant_correct = check_ant_encoder();

        if (ant_correct) //if(antenna applied)
        {
            state = 1;
            flagset = false;
        }
        break;
    case 1:
        if(!flagset){
            Serial.println("case 1");
            mp3Player.loopFolder(ESA);
            fill_display(BACKGROUND_COLOR);
            init_sliding_bars();
            flagset = true;
        }
        encoder_triggered = waitForTouchorEncoderEvent(&raw);

        solved = sliding_bars(encoder_triggered, raw, 0);

        if(solved)
        {
            state = 2;
            flagset = false;
            solved = false;
        }            
        break;

    case 2:
        if(!flagset){
            mp3Player.loopFolder(Ferdi);
            
            init_rect();
            flagset = true;
        }
        
        if(waitForTouchorEncoderEvent(&raw) < 4)
        {
            solved = rectangles_game(raw);  
            
            if(solved)
            {
                state = 3;
                flagset = false;
            }
        }
        break;

    case 3:
        if(!flagset){
            // oder nur playFolder    
            mp3Player.loopFolder(goodNews);        
            final_screen();
            flagset = true;
            //mp3Player.stop();
        }
        break;
    }
    // check if volume has been changed (by triggering volume controller)
    if(check_vol_encoder(&new_vol))
    {
        mp3Player.volume(new_vol);
    }
}
