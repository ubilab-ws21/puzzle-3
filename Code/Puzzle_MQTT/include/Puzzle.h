/*
 * Header file for function-prototypes
 */

#ifndef Puzzle_h
#define Puzzle_h


// Speed of Serial Communication
#define SERIAL_COMSPEED 115200

// Credentials for network SSID and PWD
#define SSID "ubilab"
//inset pasword for wifi here..
#define PWD "3"

// Device can be found on the network using this name
#define NAME "Puzzle_3"

// PWD for OTA
#define OTA_PWD "3"

// TCP Client connecting to this server
// IP or DNS name of TCP Server
#define SERVER_IP "192.168.1.113"
// Port of server, should be >= 2000
#define SERVER_PORT 2000
WiFiClient client;

// MQTT server or DNS
#define MQTT_SERVER_IP "192.168.1.113"
// Standard port for MQTT
#define MQTT_PORT 1883
// RAW TCP client and pubsub class using it
WiFiClient mqttClient;
PubSubClient mqtt(mqttClient);

// Global cstring for message building
#define MSG_SIZE 50
char msg[MSG_SIZE] = {'\0'};

// Enum and global variable for puzzle state
enum PuzzleState {
  idle = 0,
  active = 1,
  solved = 2,
};
PuzzleState puzzleState = idle;

// Enums for puzzle parts
enum antenna {
  missing = 0, 
  plugged = 1,
  correct = 2,
};

enum timeZones {
  noneCorrect = 0,
  oneCorrect = 1,
  twoCorrect = 2,
  threeCorrect = 3,
};

enum screenPuzzle {
  screenPuzzleUnsolved = 0,
  screenPuzzleSolved = 1,
};

// JSON dict of given size
StaticJsonDocument<2 * MSG_SIZE> dict;

//function prototypes
void mqttCallback(char*, byte*, unsigned int);
void setupOTA();
const char * puzzleStateToStr(PuzzleState);
void puzzleStateChanged();
void puzzleActive();
void puzzleSolved();
void puzzleIdle();
const char * handleMsg(const char *);
void handleStream(Stream *);

#endif