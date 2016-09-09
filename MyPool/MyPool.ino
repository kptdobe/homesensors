#include <MySensor.h>
#include <SPI.h>
#include <EEPROM.h>
#include "MyConfig.h"
#include "Structures.h"
#include "Config.h"

/***
 * Main setup
 */
void setup() { 
  
  gateway.begin( incomingMessage, NODE_ID );
  gateway.sendSketchInfo(SN, SV);
  
  #ifdef DEBUG_PROCESS
    Serial.print("Startup - initialized");
    Serial.println();
    Serial.print("SN: ");
    Serial.print(SN);
    Serial.println();
    Serial.print("SV: ");
    Serial.print(SV);
    Serial.println();
    Serial.print("Node id: ");
    Serial.print(NODE_ID);
    Serial.println();
  #endif

  gateway.present(poolLight.childId, S_LIGHT);
  pinMode(poolLight.pin, OUTPUT);
  digitalWrite(poolLight.pin, gateway.loadState(poolLight.childId)?RELAY_ON:RELAY_OFF);
}

/***
 *  Main loop
 */
void loop() {
  gateway.process();
  
}

void incomingMessage(const MyMessage &message) {
  #ifdef DEBUG_MSG
    Serial.print("Received message from gateway (type / data / sensor): ");
    Serial.print(message.type);
    Serial.print(" / ");
    Serial.print(message.data);
    Serial.print(" / ");
    Serial.print(message.sensor);
    Serial.println();
  #endif

   if (message.sensor == poolLight.childId) {
     // Change relay state
     digitalWrite(poolLight.childId, message.getBool()?RELAY_ON:RELAY_OFF);
     // Store state in eeprom
     gateway.saveState(message.sensor, message.getBool());
     // Write some debug info
     Serial.print("Incoming change for sensor:");
     Serial.print(message.sensor);
     Serial.print(", New status: ");
     Serial.println(message.getBool());
   } 
}

