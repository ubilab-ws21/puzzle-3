#include <Puzzle.h>
#include <privateInfo.h>

StaticJsonDocument<300> doc;
JsonObject JSONencoder = doc.to<JsonObject>();

// Setup function that runs once upon startup or reboot
void setup() {
  // Initialize the Serial Port
  Serial.begin(SERIAL_COMSPEED);

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
    mqtt.subscribe("3/#");
  } else {
    Serial.println("Cannot connect to MQTT server");
  }

  // Indicate end of setup
  Serial.println("Setup done!");

}


void loop() {
  // Needs to be called to check for external update request
  ArduinoOTA.handle();
  // Checks for MQTT subscriptions
  mqtt.loop();

  if (gameState == idle) {
     puzzleIdle();
  }
  else if (gameState == antenna) {
     puzzleAntenna();
  } 
  else if(gameState == mapgame) {
     puzzleMap();
  }
  else if(gameState == touchgame){
    puzzleTouchgame();
  }

  // If at least one byte is available over serial
  if (Serial.available()) {
    handleStream(&Serial);
  }
  
  // If at least one byte is available over TCP
  if (client.available()) {
    handleStream(&client);
  }
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
    puzzleSolved();
  } else if ( (strcmp(topic,"3/gamecontrol/antenna") == 0) && (strcmp(msg, "off") == 0)) {
    puzzleIdle(); //maybe different idle state required
  } else if ( (strcmp(topic,"3/gamecontrol/antenna") == 0) && (strcmp(msg, "on") == 0)) {
    puzzleAntenna();
    mqtt_publish("game/puzzle3/Antenna","trigger","active");
  } else if ((strcmp(topic,"3/gamecontrol/map") == 0) && (strcmp(msg, "off") == 0)){
    puzzleIdle(); //maybe different idle state required
  } else if ((strcmp(topic,"3/gamecontrol/map") == 0) && (strcmp(msg, "on") == 0)) {
    puzzleMap();
  } else if ((strcmp(topic,"3/gamecontrol/touchgame") == 0) && (strcmp(msg, "off") == 0)){
    puzzleIdle(); //maybe different idle state required
  } else if ((strcmp(topic,"3/gamecontrol/touchgame") == 0) && (strcmp(msg, "on") == 0)) {
    puzzleTouchgame();
  } else if (strcmp(msg, "idle") == 0) {
    puzzleIdle();
  } else {
    return "Unknown command";
  }
  return "";
}

/*
   Puzzle changes to idle state
*/
void puzzleIdle() {
  gameState = idle;
  // Code to idle puzzle ...
  //Serial.println("We are now in idle state");
  puzzleStateChanged();
}

/*
   Puzzle changes to solved state
*/
void puzzleSolved() {
  gameState = solved;
  // Code to set puzzle into solved state ...
  Serial.println("We are now in solved state");
  puzzleStateChanged();
}

/*
   Puzzle changes to antenna game state
*/
void puzzleAntenna() {
  gameState = antenna;
  // Code to set puzzle into active state ...
  Serial.println("We are now in Antennagame state");
  puzzleStateChanged();
  Serial.println("Surprisingly we just solved this");
  mqtt_publish("3/gamecontrol/antennna","status","solved");
  gameState = idle;
}

/*
   Puzzle changes to map game state
*/
void puzzleMap() {
  gameState = mapgame;
  // Code to set puzzle into active state ...
  Serial.println("We are now in Mapgame state");
  puzzleStateChanged();
  Serial.println("Surprisingly we just solved this");
  mqtt_publish("3/gamecontrol/map","status","solved");
  gameState = idle;
}

/*
   Puzzle changes to touchgame game state
*/
void puzzleTouchgame() {
  gameState = touchgame;
  // Code to set puzzle into active state ...
  Serial.println("We are now in Touchgame state");
  puzzleStateChanged();
  Serial.println("Surprisingly we just solved this");
  mqtt_publish("3/gamecontrol/touchgame","status","solved");
  gameState = idle;
}

/*
   Indicate to others that the state of the
   puzzle changed
*/
void puzzleStateChanged() {
  //Serial.printf("Puzzle state changed to: %s\n", puzzleStateToStr(gameState));

  // Convert puzzlestate into JSON dict
  JsonObject obj = dict.to<JsonObject>();
  obj.clear();
  obj["state"] = puzzleStateToStr(gameState);
  // Serialize into msg cstring
  serializeJson(obj, msg);

  // If the TCP client is connected to the server, send JSON
  if (client.connected()) {
    client.println(msg);
  }
  // If the MQTT client is connected to broker, send JSON
  if (mqtt.connected()) {
    // Specify if msg should be retained
    bool retained = true;
    // Publish msg under given topic
    mqtt.publish("myPuzzle/status", "test"); //"test", retained);                                                                                                                                                                                                                                                                                                                                           , retained);
  }

}

/*
   Convert the ENUM to a string
*/
const char * puzzleStateToStr(gamecontrol puzzle) {
  switch (puzzle) {
    case antenna: return "antenna";
    case idle: return "idle";
    case solved: return "solved";
    case mapgame: return "map";
    case touchgame: return "touchgame";
    default: return "Unknown state";
  }
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
    else if(hintcount == SOLVED){
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
    else if(hintcount == SOLVED){
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
      else if(hintcount == SOLVED){
        mqtt_publish(touchTopic,"trigger","solved");
      }
  default:
    break;
  }
}