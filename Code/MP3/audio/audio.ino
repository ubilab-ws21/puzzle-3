#include <ESPmDNS.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>


#define TX 37 //GPIO23
#define RX 36 //GPIO22

// RX/TX port definition
SoftwareSerial DFPlayerSoftwareSerial(TX, RX);

// Name of Player
DFRobotDFPlayerMini mp3Player;

// Number of Folder. Folder must contain data
int folder = 1; 


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

  // detect number of files in folder and show
  int numberFiles = mp3Player.readFileCountsInFolder(folder);

  // if no files are in the folder
  if (numberFiles == -1){
    Serial.println("Folder doesn't contain any music files!");  
    Serial.println("Closing the Program!");
    while(1);
  } else {
    Serial.print(String(numberFiles) + " Dateien im Ordner " + String(folder));
    mp3Player.loopFolder(folder); 
  }
}

void loop(){
  
}
