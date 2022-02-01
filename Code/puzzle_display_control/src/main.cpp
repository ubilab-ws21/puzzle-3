#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include "display_setup.h"
#include "display_stuff.h"
#include "login_puzzle.h"
#include "frequency_puzzle.h"
#include "encoder_handle.h"
#include <Puzzle.h>
#include <privateInfo.h>

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
    antenna_Level1 = 10,
    antenna_Level2 = 10,
    antenna_Level3 = 10,
    antenna_Level4 = 10 
};

int enc_num_triggered = 0;

bool ant_correct = false;

int new_vol = 20;
tsPoint_t raw;
int ant_value;

// the current state and the next state.
// the next state is only needed if we temporarily need to go back to the state stateAntenna. 
// Afterwards, we want to go back to the state we were in before (i.e. the state saved in next_sate)
GameState state;
GameState next_state;

// the current and the old antenna state (old one is needed for comparison)
AntennaState ant_state;
AntennaState old_ant_state;

int noise_arrays[5] = {antenna_Level0, antenna_Level1, antenna_Level2, antenna_Level3, antenna_Level4};

//MQTT implementation stuff begins here
StaticJsonDocument<300> doc;
JsonObject JSONencoder = doc.to<JsonObject>();
#define MQTT
#define OPERATORCONTROL

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

    state = stateAntenna;    
    next_state = stateFrequency;

    ant_state = antenna_Level4;
    old_ant_state = antenna_Level4;
#ifdef MQTT
    /*********************
       WiFi Connection
    *********************/
    Serial.print("Connecting to ");
    Serial.println(SSID);
    // Set name passed to AP while connection
    WiFi.setHostname(NAME);
    // Connect to AP
    WiFi.begin(SSID, PWD);
    // Wait while not connected
    while (WiFi.status() != WL_CONNECTED) {
        delay(500); Serial.print(".");
    }
    // Print IP
    Serial.println("\nWiFi connected.\nIP Adress: ");
    Serial.println(WiFi.localIP());

    /*********************
         multicast DNS
    *********************/
    // Set name of this device
    if (!MDNS.begin(NAME)) {
        Serial.println("Setting up MDNS responder!");
    }
    // The service this device hosts (example)
    MDNS.addService("_escape", "_tcp", 2000);

    /*************************
         Connect to TCP server
    *************************/
    if (client.connect(SERVER_IP, SERVER_PORT)) {
        Serial.println("Connected to server");
    } else {
        Serial.println("Cannot connect to server");
    }

    // OTA setup in separate function
    setupOTA();


    /*************************
         MQTT Setup
    *************************/
    // Set IP and port of broker
    mqtt.setServer(MQTT_SERVER_IP, MQTT_PORT);
    // Set CB function to be called for all subscriptions
    mqtt.setCallback(mqttCallback);
    // Connect to broker using a unique ID (here the name)
    if (mqtt.connect(NAME)) {
        Serial.println("Connected to MQTT server");
        // Subscribe to a certain topic
        //#TODO: add/put correct Topiclist here 
        mqtt.subscribe("3/#");
    } else {
        Serial.println("Cannot connect to MQTT server");
    }
#endif
    // Indicate end of setup
    Serial.println("Setup done!");
}

/**************************************************************************/
/*!
*/
/**************************************************************************/
void loop()
{
    #ifdef MQTT
    // Needs to be called to check for external update request
    ArduinoOTA.handle();
    // Checks for MQTT subscriptions
    mqtt.loop();
    #endif
    main_state_machine();
}

/*
to enable/disable own gamestate-control, it should be sufficient to en/disable the state-statements
this is done with defines
*/

bool secondHint = false;
char sendHint = 1;
unsigned int startTimeMap = 0;
unsigned int startTimeTouch = 0;
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
            mp3Player.loopFolder(noise_arrays[4]);
            first_screen();
            flagset = true;   
        }
        
        unsigned int ant_value;
        ant_value = check_ant_encoder();

        if(check_correct_antenna_pos(ant_value))
        {
            #ifndef OPERATORCONTROL
            state = stateFrequency;
            #endif
            flagset = false;
            #ifdef MQTT
            mqtt_publish("3/gamecontrol/antenna","state","solved");         //send to operator
            mqtt_publish("game/puzzle3/antenna","trigger","solved");        //send to hintsystem
            #endif
        }
        else
        {
            if(ant_state != old_ant_state)
            {
                mp3Player.loopFolder(ant_state);
                old_ant_state = ant_state;
            }
        }
        break;
    case stateFrequency:
        if(!flagset){
            Serial.println("case 1");
            //mp3Player.loopFolder(ESA);
            fill_display(BACKGROUND_COLOR);
            init_sliding_bars();
            flagset = true;
            startTimeMap = millis();
            
        }
        
        if(check_touch_or_encoder_events())
        {
            solved = sliding_bars(enc_num_triggered, raw, 0);
        }

        if((millis() - startTimeMap > 120000) && (secondHint == false)){
            secondHint = true;
            #ifdef MQTT
            mqtt_publish("game/puzzle3/map","trigger","active_2");
            #endif
        }
        
        if(solved)
        {
            #ifndef OPERATORCONTROL
            state = stateLogin;
            #endif
            flagset = false;
            solved = false;
            #ifdef MQTT
            mqtt_publish("3/gamecontrol/map","state","solved");         //send to operator
            mqtt_publish("game/puzzle3/map","trigger","solved");        //send to hintsystem
            #endif
        }       
        break;

    case stateLogin:
        if(!flagset){
            //mp3Player.loopFolder(Ferdi);
            init_rect();
            flagset = true;
            startTimeTouch = millis();
            
        }
        
        if(check_touch_or_encoder_events())
        {
            solved = rectangles_game(raw);
        }

        #ifdef MQTT
        if((millis() - startTimeTouch > 120000) && (sendHint == 1)){     //send second hint after 2 minutes
            sendHint = 2;
            mqtt_publish("game/puzzle3/touchgame","trigger","active_2");
        }

        if((millis() - startTimeTouch > 240000) && (sendHint == 2)){     //send third hint after 4 minutes
            sendHint = 3;
            mqtt_publish("game/puzzle3/touchgame","trigger","active_3");
        }

        if((millis() - startTimeTouch > 360000) && (sendHint == 3)){     //send fourth hint after 6 minutes
            sendHint = 4;
            mqtt_publish("game/puzzle3/touchgame","trigger","active_4");
        }
        #endif

        if(solved)
        {
            #ifndef OPERATORCONTROL
            state = stateDone;
            #endif
            #ifdef MQTT
            mqtt_publish("3/gamecontrol/touchgame","state","solved");       //send to operator
            mqtt_publish("game/puzzle3/touchgame","trigger","solved");      //send to hintsystem
            #endif
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


/*
   Handles incoming data of a stream
   This can be any stream compliant class such as
   Serial, WiFiClient (TCP or UDP), File, etc.
*/
void handleStream(Stream * getter) {
  // Copy incoming data into msg cstring until newline is received
  snprintf(msg, MSG_SIZE, "%s", getter->readStringUntil('\n').c_str());
  // Some systems also send a carrige return, we want to strip it off
  for (size_t i = 0; i < MSG_SIZE; i++) {
    if (msg[i] == '\r') {
      msg[i] = '\0';
      break;
    }
  }
  // Handle given Msg
  const char * returnMsg = handleMsg(msg, msg); //not sure what happens here, 
  // Write something back to stream only if wanted
  if (returnMsg[0] != '\0') {
    getter->println(returnMsg);
  }
}

/*
   Handle a received message, may return some answer.
*/
const char * handleMsg(const char * msg, const char* topic) {
  // strcmp returns zero on a match
  if (strcmp(msg, "solved") == 0) {
    //puzzleSolved();
    state = stateDone;
  } else if ( (strcmp(topic,"3/gamecontrol/antenna") == 0) && (strcmp(msg, "off") == 0)) {
    //puzzleIdle(); //maybe different idle state required
    flagset = false;
    state = stateIdle;
    Serial.println("Idle");
  } else if ( (strcmp(topic,"3/gamecontrol/antenna") == 0) && (strcmp(msg, "on") == 0)) {
    //puzzleAntenna();
    flagset = false;
    state = stateAntenna;
    Serial.println("Antenna");
    mqtt_publish("3/gamecontrol/antenna","state","active");
    mqtt_publish("game/puzzle3/antenna","trigger","active_1");  //release first hint for the antenna game
  } else if ((strcmp(topic,"3/gamecontrol/map") == 0) && (strcmp(msg, "off") == 0)){
    //puzzleIdle(); //maybe different idle state required
    flagset = false;
    state = stateIdle;
  } else if ((strcmp(topic,"3/gamecontrol/map") == 0) && (strcmp(msg, "on") == 0)) {
    //puzzleMap();
    Serial.println("Map");
    mqtt_publish("3/gamecontrol/map","state","active");
    mqtt_publish("game/puzzle3/map","trigger","active_1");  //release first hint for the map/frequency game
    flagset = false;
    state = stateFrequency;
  } else if ((strcmp(topic,"3/gamecontrol/touchgame") == 0) && (strcmp(msg, "off") == 0)){
    //puzzleIdle(); //maybe different idle state required
    flagset = false;
    state = stateIdle;
  } else if ((strcmp(topic,"3/gamecontrol/touchgame") == 0) && (strcmp(msg, "on") == 0)) {
    //puzzleTouchgame();
    mqtt_publish("3/gamecontrol/touchgame","state","active");
    mqtt_publish("game/puzzle3/touchgame","trigger","active_1");
    flagset = false;
    state = stateLogin;
    Serial.println("Touch");
  } else if (strcmp(msg, "idle") == 0) {
    //puzzleIdle();
    flagset = false;
    state = stateIdle;
  } else {
    return "Unknown command";
  }
  return "";
}

/*
   Setup OTA Updates
*/
void setupOTA() {
  // Set name of this device (again)
  ArduinoOTA.setHostname(NAME);
  // Set a password to protect against uploads from other parties
  ArduinoOTA.setPassword(OTA_PWD);

  // CB function for when update starts
  ArduinoOTA.onStart([]() {
    Serial.println("Start updating");
  });
  // CB function for when update starts
  ArduinoOTA.onEnd([]() {
    Serial.println("Finished updating");
  });
  // CB function for progress
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Update at %u % \n", progress / (total / 100));
  });
  // CB function for when update was interrupted
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.println("Error updating");
    ESP.restart();
  });
  // Start OTA update service
  ArduinoOTA.begin();
}

/*
 * Callback for MQTT messages on subscribed topics
 */
void mqttCallback(char* topic, byte* message, unsigned int length) {
  // Convert message as byte array to cstring
  unsigned int len = min((int)length, (int)(MSG_SIZE - 1));
  memcpy(&msg[0], message, len);
  msg[len] = '\0';
  // Print over serial
  Serial.printf("MQTT msg on topic: %s: %s\n", topic, &msg);

  // Try to deserialize the JSON msg
  DeserializationError error = deserializeJson(dict, msg);
  // Test if parsing succeeds.
  if (error) {
    Serial.println("error deserializing the msg");
  } else {
    Serial.println("Msg is of valid JSON format");
    // Try to extract new state
    const char* newState = dict["state"];
    // If key exists this is not NULL
    Serial.println(newState);
    if (newState) {
      handleMsg(newState, topic);
    }
  }
}

void mqtt_publish(const char* topic, const char* method, const char* state){
  JSONencoder["method"] = method;
  JSONencoder["state"] = state;
  JSONencoder["data"] = 0;

  char JSONmessageBuffer[100];

  serializeJson(doc,JSONmessageBuffer, 100);
  mqtt.publish(topic, JSONmessageBuffer,true); //"test", retained);
}

void publish_Hint(char game, char hintcount){
  const char antennaTopic[] = "game/puzzle3/antenna";
  const char mapTopic[] = "game/puzzle3/map";
  const char touchTopic[] = "game/puzzle3/touchgame";
  switch (game)
  {
  case 1:
    if(hintcount == FIRSTHINT){
      mqtt_publish(antennaTopic,"trigger","active_1");
    }
    else if(hintcount == INACTIVE){
      mqtt_publish(antennaTopic,"trigger","inactive");
    }
    else if(hintcount == HSOLVED){
      mqtt_publish(antennaTopic,"trigger","solved");
    }
    break;
  case 2:
    if(hintcount == FIRSTHINT){
      mqtt_publish(mapTopic,"trigger","active_1");
    }
    else if(hintcount == SECONDHINT){
      mqtt_publish(mapTopic,"trigger","active_2");
    }
    else if(hintcount == INACTIVE){
      mqtt_publish(mapTopic,"trigger","inactive");
    }
    else if(hintcount == HSOLVED){
      mqtt_publish(mapTopic,"trigger","solved");
    }
    case 3:
      if(hintcount == FIRSTHINT){
        mqtt_publish(touchTopic,"trigger","active_1");
      }
      else if(hintcount == SECONDHINT){
        mqtt_publish(touchTopic,"trigger","active_2");
      }
      else if(hintcount == THIRDHINT){
        mqtt_publish(touchTopic,"trigger","active_3");
      }
      else if(hintcount == FOURTHHINT){
        mqtt_publish(touchTopic,"trigger","active_4");
      }
      else if(hintcount == INACTIVE){
        mqtt_publish(touchTopic,"trigger","inactive");
      }
      else if(hintcount == HSOLVED){
        mqtt_publish(touchTopic,"trigger","solved");
      }
  default:
    break;
  }
}