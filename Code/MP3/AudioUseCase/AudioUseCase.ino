#include <ESPmDNS.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

void setupTest();
void statusChange();
void volumeControl();


#define TX 22 
#define RX 23 

#define ESA 1       // folder number of Emergency Message
#define Ferdi 2     // folder number of Radio station message 
#define goodNews 3  // folder number of good News Radio
// RX/TX port definition

int state = 0;   // is to be updated if gamestate changes

SoftwareSerial DFPlayerSoftwareSerial(TX, RX);

// Name of Player
DFRobotDFPlayerMini mp3Player;

void setup(){
  DFPlayerSoftwareSerial.begin(9600);
  Serial.begin(9600);

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


}

void loop(){
  state = 0;
  statusChange(state);
}



//___________Function Definitions______________

void statusChange(int state){
    switch (state){
      case 0:
              setupTest();
              state = 10;
              break;
      case 1:
              //play ESA Broadcast
              mp3Player.loopFolder(ESA);
              state = 10;
              break;
      case 2:
              //play Ferdi Radio
              mp3Player.loopFolder(Ferdi);
              state = 10;
              break;
      case 3: 
              //play Happy Radio
              mp3Player.loopFolder(goodNews);
              state = 10;
              break;
      case 10:
              //playback as it is, waiting for new state update
              break;
      default: 
              break;             
  }
}

void volumeControl(){
  
}


void setupTest(){
   // detect number of files in folder and show
    int numberFiles = mp3Player.readFileCountsInFolder(ESA);
  
    // if no files are in the folder
    if (numberFiles == -1){
      Serial.println("Folder doesn't contain any music files!");  
      Serial.println("Closing the Program!");
      while(1);
    } else {
      Serial.print(String(numberFiles) + " Dateien im Ordner " + String(ESA));
      mp3Player.loopFolder(ESA);
    }
}
