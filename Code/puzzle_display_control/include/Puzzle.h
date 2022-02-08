/*
 * Header file for function-prototypes
 */
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#ifndef Puzzle_h
#define Puzzle_h

//macros for hintsystem
#define HSOLVED  10
#define FIRSTHINT 1
#define SECONDHINT 2
#define THIRDHINT 3
#define FOURTHHINT 4
#define INACTIVE 0

// Speed of Serial Communication
#define SERIAL_COMSPEED 115200


// Device can be found on the network using this name
#define NAME "Puzzle_3"

// PWD for OTA
#define OTA_PWD "3"

// TCP Client connecting to this server
// IP or DNS name of TCP Server
#define SERVER_IP "10.8.166.20"
// Port of server, should be >= 2000
#define SERVER_PORT 2000
WiFiClient client;

// MQTT server or DNS
#define MQTT_SERVER_IP "10.8.166.20"
// Standard port for MQTT
#define MQTT_PORT 1883
// RAW TCP client and pubsub class using it
WiFiClient mqttClient;
PubSubClient mqtt(mqttClient);

// Global cstring for message building
#define MSG_SIZE 100
char msg[MSG_SIZE] = {'\0'};

// Enum and global variable for puzzle game control
/*enum gamecontrol {
  idle = 0,
  antenna = 1,
  antennaSolved = 2,
  mapgame = 3,
  mapSolved = 4,
  touchgame = 5,
  touchgameSolved = 6,
  solved = 7,
};
enum gamecontrol gameState = idle;*/



// JSON dict of given size
StaticJsonDocument<2 * MSG_SIZE> dict;

//function prototypes
void mqttCallback(char*, byte*, unsigned int);
void setupOTA();
//const char * puzzleStateToStr(gamecontrol);
//void puzzleStateChanged();
//void puzzleActive();
/* void puzzleSolved();
void puzzleIdle();
void puzzleAntenna();
void puzzleMap();
void puzzleTouchgame(); */
const char * handleMsg(const char *, const char *);
void handleStream(Stream *);
void mqtt_publish(const char*, const char*, const char*);
void publish_Hint(int, int);
#endif