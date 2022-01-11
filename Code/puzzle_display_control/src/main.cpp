#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include "display_setup.h"
#include "display_stuff.h"
#include "encoder_handle.h"

#include <stdio.h>
#include <stdlib.h>

#define LETTER_GAME 1


// Audio interface
#define TX 4 
#define RX 15 

#define ESA 1       // folder number of Emergency Message
#define Ferdi 2     // folder number of Radio station message 
#define goodNews 3  // folder number of good News Radio

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
    mp3Player.volume(20);

    init_display();

  //tft.setFont(&FreeSerifBoldItalic9pt7b);
    init_encoder();

    

//#ifndef LETTER_GAME
    sliding_bars(1);
    sliding_bars(2);
    sliding_bars(3);
//#else
 //   init_rect();
//#endif

    state = 1;
    
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
        switch(state)
        {
        case 1:
            if(!flagset){
                mp3Player.loopFolder(ESA);
                sliding_bars(1);
                sliding_bars(2);
                sliding_bars(3);
                flagset = true;
            }
            encoder_triggered = check_encoder();
            if (encoder_triggered)
            {
                solved = sliding_bars(encoder_triggered);
            }
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
            waitForTouchorEncoderEvent(&raw);
            solved = rectangles_game(raw);  
            
            if(solved)
            {
                state = 3;
                flagset = false;
            }
            break;

            case 3:
                final_screen();
                //whitescreen;
                //pause_audio;
                break;
        }
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