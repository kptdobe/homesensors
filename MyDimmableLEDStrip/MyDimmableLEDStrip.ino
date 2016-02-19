#include <MySensor.h>
#include <SPI.h>
#include <EEPROM.h>
#include "MyConfig.h"
#include "Structures.h"
#include "Config.h"

/*
 * GATEWAY 
 */
#define SN "MotionDimmableLEDStrip"
#define SV "1.1"
#define NODE_ID 10

MySensor gateway;

/*
 * LED
 */

COLOR red = {
  103,
  3,
  0
};

COLOR green = {
  105,
  5,
  0
};

COLOR blue = {
  106,
  6,
  0
};

RGB rgb = {
  red,
  green,
  blue
};

/* 
 *  MOTION SENSOR  
 */
#define INTERRUPT MOTION_PIN-2

MOTION motion = {
  110,
  2,
  false,
  false,
  false
};

MyMessage motionMsg(motion.childId, V_TRIPPED);

/*
 * PHOTOCELL SENSOR
 */

PHOTOCELL photocell = {
  0,      // pin: A0
  1500,   // level: max allowed level
  0       // last: last read value
};

void saveRGB() {
  EEPROM.write(10, rgb.red.level);
  EEPROM.write(11, rgb.green.level);
  EEPROM.write(12, rgb.blue.level);

  #ifdef DEBUG_EEPROM
    Serial.print("Saved RGB levels to EEPROM (R / G / B): ");
    Serial.print(rgb.red.level);
    Serial.print(" / ");
    Serial.print(rgb.green.level);
    Serial.print(" / ");
    Serial.print(rgb.blue.level);
    Serial.println();
  #endif
}

void loadRGB() {
  rgb.red.level = EEPROM.read(10);
  rgb.green.level = EEPROM.read(11);
  rgb.blue.level = EEPROM.read(12);
  
  #ifdef DEBUG_EEPROM
    Serial.print("Read RGB levels to EEPROM (R / G / B): ");
    Serial.print(rgb.red.level);
    Serial.print(" / ");
    Serial.print(rgb.green.level);
    Serial.print(" / ");
    Serial.print(rgb.blue.level);
    Serial.println();
  #endif
}

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
  
  /*
   * LEDs: setup
   */

  // register the LEDs as dimmable lights with the gateway
  gateway.present( rgb.red.childId, S_DIMMER );
  gateway.present( rgb.green.childId, S_DIMMER );
  gateway.present( rgb.blue.childId, S_DIMMER );
  
  pinMode(rgb.red.pin, OUTPUT);
  pinMode(rgb.green.pin, OUTPUT);
  pinMode(rgb.blue.pin, OUTPUT);

  // initial white flash
  on(true);
  delay(500);
  off();

  // load EEPROM persisted valus
  loadRGB();

  // initial colored test flash
  on();
  delay(500);
  off();

  /*
   * MOTION: setup
   */
   
  pinMode(motion.pin, INPUT);
  gateway.present(motion.childId, S_MOTION);
}

/***
 *  Main loop
 */
void loop() 
{
  gateway.process();

  handleLight();
}

void handleLight() {

  if (motion.forced) {
    if (!isLightOn()) {
      #ifdef DEBUG_PROCESS
        Serial.print("Turning ON because motion set to forced mode.");
        Serial.println();
      #endif
      
      on();
      return;
    } else {
      // do nothing because motion is forced and light is already on
      return;
    }
  }
  
  int photocellReading = analogRead(photocell.pin);
  boolean isCell = (photocellReading <= photocell.level);

  if (!isCell) {
    if (isLightOn()) {
      #ifdef DEBUG_PROCESS
        Serial.print("Turning OFF because photocell value is below level (current / level): ");
        Serial.print(photocellReading);
        Serial.print(" / ");
        Serial.print(photocell.level);
        Serial.println();
      #endif
      
      off();
      return;
    } else {
      // do nothing because luminosity is to high
      return;
    }
  }

  // read digital motion value
  motion.current = (digitalRead(motion.pin) == HIGH);
  
  if (motion.current && !isLightOn()) {
    #ifdef DEBUG_PROCESS
      Serial.print("Turning ON because motion sensor detected move.");
      Serial.println();
    #endif
    
    on();
    //gateway.send(motionMsg.set(motion.current?"1":"0"));  // send motion value to gateway
    
    return;
  }

  if (!motion.current && isLightOn()) {
    #ifdef DEBUG_PROCESS
      Serial.print("Turning OFF because motion sensor stop detecting move.");
      Serial.println();
    #endif
    
    //gateway.send(motionMsg.set(motion.current?"1":"0"));  // send motion value to gateway
    off();
    
    return;
  }
}


void incomingMessage(const MyMessage &message) {
    #ifdef DEBUG_MSG
      Serial.print("Received message from gateway (type / data / sensor)");
      Serial.print(message.type);
      Serial.print(" / ");
      Serial.print(message.data);
      Serial.print(" / ");
      Serial.print(message.sensor);
      Serial.println();
    #endif
      
    if (message.type == V_DIMMER) {
    
    //  Retrieve the power or dim level from the incoming request message
    int requestedLevel = atoi( message.data );
    
    // Adjust incoming level if this is a V_LIGHT variable update [0 == off, 1 == on]
    requestedLevel *= ( message.type == V_LIGHT ? 100 : 1 );
    
    // Clip incoming level to valid range of 0 to 100
    requestedLevel = requestedLevel > 100 ? 100 : requestedLevel;
    requestedLevel = requestedLevel < 0   ? 0   : requestedLevel;

    COLOR* color = getColor(message.sensor);

    if (color != NULL) {
      int value = (int)(requestedLevel / 100. * 255);
      #ifdef DEBUG_MSG
        Serial.print("Interpreted color change request (% / from / to / pin): ");
        Serial.print(requestedLevel);
        Serial.print(" / ");
        Serial.print(color->level);
        Serial.print(" / ");
        Serial.print(value);
        Serial.print(" / ");
        Serial.print(color->pin);
        Serial.println();
      #endif
      
      setLevel(color, value);
      on();
      delay(500);
      off();
    } else {
      #ifdef DEBUG_MSG
        Serial.print("Interpreted color change request but INVALID color sensor");
        Serial.println();
      #endif
    }
  } else {
    if (message.type == V_LIGHT && message.sensor == motion.childId) {
      motion.forced = (atoi( message.data ) == 1);
      
      #ifdef DEBUG_MSG
        Serial.print("Interpreted motion force request (data): ");
        Serial.print(motion.forced);
        Serial.println();
      #endif
    }
  }
}

void setLevel(COLOR* color, int level) {
  level = level >= 0 && level <= 255 ? level : 0;
  color->level = level;
  saveRGB();
}

COLOR* getColor(int childId) {
  if ( childId == rgb.red.childId) {
    return &rgb.red;
  }
  if ( childId == rgb.green.childId) {
    return &rgb.green;
  }
  if ( childId == rgb.blue.childId) {
    return &rgb.blue;
  }

  return NULL;
}

boolean isOn = false;

boolean isLightOn() {
  return isOn;
}

void on() {
  on(false);
}

void on(boolean force) {
  if (!isOn) {
    isOn = true;
    on(&rgb.red, force);
    on(&rgb.green, force);
    on(&rgb.blue, force);
  }
}

void off() {
  if (isOn) {
    off(&rgb.red);
    off(&rgb.green);
    off(&rgb.blue);
    isOn = false;
  }
}

void on(COLOR* color, boolean force) {
  #ifdef DEBUG_COLOR
    Serial.print("Turning ON color (pin / level / forced): ");
    Serial.print(color->pin);
    Serial.print(" / ");
    Serial.print(color->level);
    Serial.print(" / ");
    Serial.print(force);
    Serial.println();
  #endif
  
  analogWrite( color->pin, force ? 255 : color->level );
}

void off(COLOR* color) {
  #ifdef DEBUG_COLOR
    Serial.print("Turning OFF color on pin ");
    Serial.print(color->pin);
    Serial.println();
  #endif
  
  analogWrite( color->pin, 0);
}

