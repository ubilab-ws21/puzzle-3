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
#define HIGH_NOISE 10
#define NOISE 10    // folder number of noise
#define LOW_NOISE 10
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


enum GameState{
    stateIdle = 1,
    stateAntenna,
    stateFrequency,
    stateLogin,
    stateDone
};

// the current state
GameState state;

void main_state_machine();
bool check_touch_or_encoder_events();

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

    state = stateIdle;    
}

/**************************************************************************/
/*!
*/
/**************************************************************************/
int enc_num_triggered = 0;
bool ant_correct = false;
int new_vol = 20;
tsPoint_t raw;
int ant_value;

int next_state = stateFrequency;


int ant_state = 0;
int old_ant_state = 0;

int noise_arrays[3] = {HIGH_NOISE, NOISE, LOW_NOISE};

void loop()
{
    main_state_machine();
}


void main_state_machine()
{
    switch(state)
    {
    case stateIdle:

    break;
    case stateAntenna:
        if(!flagset)
        {
            Serial.println("case 0");
            mp3Player.loopFolder(HIGH_NOISE);
            first_screen();
            flagset = true;   
        }
        
        ant_correct = check_ant_encoder(&ant_value);
        /*
        if(!ant_correct)
        {
            if(ant_value <= 0)
            {
                ant_state = 0;
            }
            else if(ant_value <= 2)
            {
                ant_state = 1;
            }
            else if(ant_value <= 5)
            {
                ant_state = 2;
            }
            if(ant_state != old_ant_state)
            {
                mp3Player.loopFolder(noise_arrays[ant_state]);
                old_ant_state = ant_state;
            }
        }
        else 
        */
        if(ant_correct)
        {
            state = stateFrequency;
            flagset = false;
        }
        break;
    case stateFrequency:
        if(!flagset){
            Serial.println("case 1");
            //mp3Player.loopFolder(ESA);
            fill_display(BACKGROUND_COLOR);
            init_sliding_bars();
            flagset = true;
        }
        
        if(check_touch_or_encoder_events())
        {
            solved = sliding_bars(enc_num_triggered, raw, 0);
        }
        
        if(solved)
        {
            state = stateLogin;
            flagset = false;
            solved = false;
        }       
        break;

    case stateLogin:
        if(!flagset){
            //mp3Player.loopFolder(Ferdi);
            init_rect();
            flagset = true;
        }
        
        if(check_touch_or_encoder_events())
        {
            solved = rectangles_game(raw);
        }
        
        if(solved)
        {
            state = stateDone;
            flagset = false;
        }
        break;

    case stateDone:
        if(!flagset){
            // oder nur playFolder    
            mp3Player.volume(25);
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
/*
    ant_correct = check_ant_encoder(&ant_value);
    if(!ant_correct)
    {
        Serial.print("ant not correct");
        flagset = false;
        next_state = state;
        state = stateAntenna;
    } */
}

bool check_touch_or_encoder_events()
{
    enc_num_triggered = check_game_encoders();
    if(enc_num_triggered)
    {
        set_last_action(enc_num_triggered);
        return true;
    }
    else if(!digitalRead(RA8875_INT))
    {
        handleTouchEvent(&raw);
        // set_last_action(4); // 4 = last_action_touch
        return true;
    }
    
    return false;
}