#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include "display_setup.h"
#include "display_stuff.h"
#include "login_puzzle.h"
#include "frequency_puzzle.h"
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

#define ANTENNA_CORRECT_POS 12

void main_state_machine();
bool check_touch_or_encoder_events();
bool check_correct_antenna_pos(unsigned int);

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

enum AntennaState{
    antenna_NoNoise = 0,
    antenna_Level0 = 10, 
    antenna_Level1 = 11,
    antenna_Level2 = 11,
    antenna_Level3 = 12,
    antenna_Level4 = 12
};

int enc_num_triggered = 0;

bool ant_correct = false;

int new_vol = 20;
tsPoint_t raw;


// the current state and the next state.
// the next state is only needed if we temporarily need to go back to the state stateAntenna. 
// Afterwards, we want to go back to the state we were in before (i.e. the state saved in next_sate)
GameState state;

// the current and the old antenna state (old one is needed for comparison)
AntennaState ant_state;
AntennaState old_ant_state;

int noise_arrays[5] = {antenna_Level0, antenna_Level1, antenna_Level2, antenna_Level3, antenna_Level4};


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
    mp3Player.volume(5);

    init_display();
    init_encoder();

    state = stateIdle;

    ant_state = antenna_Level4;
    old_ant_state = antenna_Level4;
}

/**************************************************************************/
/*!
*/
/**************************************************************************/
void loop()
{
    main_state_machine();
}

unsigned int ant_value;
static Adafruit_RA8875 local_tft =  gettft();
void main_state_machine()
{
    switch(state)
    {
    case stateIdle:
        if(!flagset)
        {
           mp3Player.stop();
           local_tft.sleep(true);
           flagset = true;
        }
        if(check_game_encoders())
        {
            state = stateAntenna;
            flagset = false;
            local_tft.sleep(false);
        }
        break;
    case stateAntenna:
        if(!flagset)
        {
            Serial.println("case 0");
            mp3Player.loopFolder(antenna_Level4);
            first_screen();
            flagset = true;   
        }
        
        ant_value = check_ant_encoder();
        if(check_correct_antenna_pos(ant_value))
        {
            state = stateFrequency;
            flagset = false;
        }
        else
        {
            if(ant_state != old_ant_state)
            {
                mp3Player.loopFolder(ant_state);
                old_ant_state = ant_state;
            }
            else{
                Serial.println("do nothing");
            }
        }
        break;
    case stateFrequency:
        if(!flagset){
            Serial.println("case 1");
            mp3Player.loopFolder(ESA);
            fill_display(BACKGROUND_COLOR);
            encoder_set_value(1, 0);
            encoder_set_value(2, 0);
            encoder_set_value(3, 0);
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
            mp3Player.loopFolder(Ferdi);
            init_rect();
            flagset = true;
        }
        
        if(check_touch_or_encoder_events())
        {
            solved = login_game(raw);
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
    if(state != stateAntenna)
    { 
        ant_value = check_ant_encoder();

        if(!check_correct_antenna_pos(ant_value))
        {
            Serial.print("ant not correct");
            flagset = false;
            state = stateAntenna;
        }
    }*/
}
int last_touched = millis();
bool check_touch_or_encoder_events()
{
    enc_num_triggered = check_game_encoders();
    
    if(!digitalRead(RA8875_INT) && millis()-last_touched > 500)
    {
        last_touched = millis();
        handleTouchEvent(&raw);
        return true;
    }
    else if(enc_num_triggered)
    {
        set_last_action(enc_num_triggered);
        return true;
    }
    else{
        set_last_action(last_action_none);
    }
    return false;
}

bool check_correct_antenna_pos(unsigned int ant_value)
{
        if(ant_value < 2)
        {
            ant_state = antenna_Level4;
        }
        else if(ant_value < 4)
        {
            ant_state = antenna_Level3;
        }
        else if(ant_value < 6)
        {
            ant_state = antenna_Level2;
        }
        else if(ant_value < 8)
        {
            ant_state = antenna_Level1;
        }
        else if(ant_value < 10)
        {
            ant_state = antenna_Level0;
        }
        else if(ant_value <= ANTENNA_CORRECT_POS)
        {
            Serial.print("correct");
            return true;
        }
        return false;
}