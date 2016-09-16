#include "Structures.h"
#include "Config.h"

#include <SPI.h>
#include <EEPROM.h>
#include <MySensors.h>

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
    Serial.print("Read RGB levels from EEPROM (R / G / B): ");
    Serial.print(rgb.red.level);
    Serial.print(" / ");
    Serial.print(rgb.green.level);
    Serial.print(" / ");
    Serial.print(rgb.blue.level);
    Serial.println();
  #endif
}

void savePhotocell() {
  int address = 13;
  int value = photocell.maxLevel;
  byte two = (value & 0xFF);
  byte one = ((value >> 8) & 0xFF);

  EEPROM.write(address, two);
  EEPROM.write(address + 1, one);
  
  #ifdef DEBUG_EEPROM
    Serial.print("Saved photocell max level to EEPROM: ");
    Serial.print(photocell.maxLevel);
    Serial.println();
  #endif
}

void loadPhotocell() {
  int address = 13;
  long two = EEPROM.read(address);
  long one = EEPROM.read(address + 1);
  
  photocell.maxLevel = ((two << 0) & 0xFF) + ((one << 8) & 0xFFFF);

  #ifdef DEBUG_EEPROM
    Serial.print("Read photocell max level from EEPROM: ");
    Serial.print(photocell.maxLevel);
    Serial.println();
  #endif
}

void saveTimer() {
  int address = 15;
  int value = timer.duration;
  byte two = (value & 0xFF);
  byte one = ((value >> 8) & 0xFF);

  EEPROM.write(address, two);
  EEPROM.write(address + 1, one);
  
  #ifdef DEBUG_EEPROM
    Serial.print("Saved timer max duration to EEPROM: ");
    Serial.print(timer.duration);
    Serial.println();
  #endif
}

void loadTimer() {
  int address = 15;
  long two = EEPROM.read(address);
  long one = EEPROM.read(address + 1);
  
  timer.duration = ((two << 0) & 0xFF) + ((one << 8) & 0xFFFF);

  #ifdef DEBUG_EEPROM
    Serial.print("Read timer max duration from EEPROM: ");
    Serial.print(timer.duration);
    Serial.println();
  #endif
}

/***
 * Main setup
 */
void setup() { 
}

void presentation() {  
  
  sendSketchInfo(SN, SV);
  
  #ifdef DEBUG_PROCESS
    Serial.print("Startup - initialized");
    Serial.println();
    Serial.print("SN: ");
    Serial.print(SN);
    Serial.println();
    Serial.print("SV: ");
    Serial.print(SV);
    Serial.println();
    Serial.println();
  #endif
  
  /*
   * LEDs: setup
   */

  pinMode(rgb.red.pin, OUTPUT);
  pinMode(rgb.green.pin, OUTPUT);
  pinMode(rgb.blue.pin, OUTPUT);

  // initial white flash
  on(true);
  wait(500);
  off();

  // load EEPROM persisted valus
  loadRGB();
  loadPhotocell();
  loadTimer();

  // initial colored test flash
  on();
  wait(500);
  off();

  /*
   * MOTION: setup
   */
   
  pinMode(motion.pin, INPUT);


  present( rgb.red.childId, S_DIMMER );
  present( rgb.green.childId, S_DIMMER );
  present( rgb.blue.childId, S_DIMMER );
  present(motion.childId, S_MOTION);
  present(photocell.childId, S_DIMMER);
  present(timer.childId, S_DIMMER);
  present(temp.childId, S_TEMP);

}

/***
 *  Main loop
 */
void loop() {
  handleTemperature();
  handleLight();
}

unsigned long lastTimeTemp;

void handleTemperature() {
  unsigned long current = millis();

  // read only every x seconds
  if(current - lastTimeTemp < TEMPERATURE_READ_TIMEOUT) return;

  lastTimeTemp = current;
  int reading = analogRead(temp.pin);  
 
  float voltage = reading * 5.0 / 1024.0;
  float temperature = (voltage - 0.5) * 100;
  
  #ifdef DEBUG_TEMPERATURE
      Serial.print("Temperature (voltage / last / current / diff): ");
      Serial.print(voltage);
      Serial.print(" / ");
      Serial.print(temperature);
      Serial.print(" / ");
      Serial.print(temp.last);
      Serial.print(" / ");
      Serial.print(abs(temperature - temp.last));
      
      Serial.println();
      wait(3000);
    #endif
 
  // Only send data if temperature has changed and no error
  if (temp.last != temperature && 
        temperature > -40.00 && 
        temperature < 125.00 &&
        abs(temperature - temp.last) > TEMPERATURE_MIN_DELTA) {
    // Send in the new temperature
    
    
    // Save new temperatures for next compare
    temp.last = temperature;
  }
  
}

unsigned long onStartTime;

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
  photocell.current = photocellReading;
  boolean isCell = (photocellReading <= photocell.maxLevel);

  #ifdef DEBUG_PHOTOCELL
      Serial.print("Photocell (current / maxLevel): ");
        Serial.print(photocellReading);
        Serial.print(" / ");
        Serial.print(photocell.maxLevel);
        Serial.println();
  #endif
  
  if (!isCell) {
    if (isLightOn()) {
      #ifdef DEBUG_PROCESS
        Serial.print("Turning OFF because photocell value is below level (current / maxLevel): ");
        Serial.print(photocellReading);
        Serial.print(" / ");
        Serial.print(photocell.maxLevel);
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
  
  if (motion.current) {
    #ifdef DEBUG_MOTION
      Serial.print("Turning ON because motion sensor detected move (duration): ");
      Serial.print(timer.duration);
      Serial.println();
    #endif

    onStartTime = millis();
    on();
    //send(motionMsg.set(motion.current?"1":"0"));  // send motion value to gateway
    
    return;
  }

  unsigned long elsapsedTime = (millis() - onStartTime) / 1000;
  if (elsapsedTime >= timer.duration && isLightOn()) {
    #ifdef DEBUG_PROCESS
      Serial.print("Turning OFF because on duration time is over (elsapsedTime / duration): ");
      Serial.print(elsapsedTime);
      Serial.print(" / ");
      Serial.print(timer.duration);
      Serial.println();
    #endif
    
    //send(motionMsg.set(motion.current?"1":"0"));  // send motion value to gateway
    off();
    
    return;
  }
}

int getValidPercentage(const MyMessage &message) {
  //  Retrieve the power or dim level from the incoming request message
  int requestedLevel = atoi( message.data );

  // Adjust incoming level if this is a V_LIGHT variable update [0 == off, 1 == on]
  requestedLevel *= ( message.type == V_LIGHT ? 100 : 1 );

  // Clip incoming level to valid range of 0 to 100
  requestedLevel = requestedLevel > 100 ? 100 : requestedLevel;
  requestedLevel = requestedLevel < 0   ? 0   : requestedLevel;

  return requestedLevel;
}

void receive(const MyMessage &message) {
  #ifdef DEBUG_MSG
    Serial.print("Received message from gateway (type / data / sensor): ");
    Serial.print(message.type);
    Serial.print(" / ");
    Serial.print(message.data);
    Serial.print(" / ");
    Serial.print(message.sensor);
    Serial.println();
  #endif

  if (message.sensor == rgb.red.childId || message.sensor == rgb.green.childId || message.sensor == rgb.blue.childId) {
  
    //  Retrieve the power or dim level from the incoming request message
    int requestedLevel = getValidPercentage(message);
  
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
      wait(500);
      off();
    } else {
      #ifdef DEBUG_MSG
        Serial.print("Interpreted color change request but INVALID color sensor");
        Serial.println();
      #endif
    }
  }

  if (message.sensor == motion.childId) {
    motion.forced = (atoi( message.data ) == 1);
    
    #ifdef DEBUG_MSG
      Serial.print("Interpreted motion force request (data): ");
      Serial.print(motion.forced);
      Serial.println();
    #endif
  }

  if (message.sensor == photocell.childId) {
    int requestedLevel = getValidPercentage(message);
    int value = (int)(requestedLevel / 100. * PHOTOCELL_MAX_RANGE); // % of PHOTOCELL_MAX_RANGE
    
    #ifdef DEBUG_MSG
      Serial.print("Interpreted photocell max level change request (% / from / to / max): ");
      Serial.print(requestedLevel);
      Serial.print(" / ");
      Serial.print(photocell.maxLevel);
      Serial.print(" / ");
      Serial.print(value);
      Serial.print(" / ");
      Serial.print(PHOTOCELL_MAX_RANGE);
      
      Serial.println();
    #endif

    photocell.maxLevel = value;

    savePhotocell();
  }

  if (message.sensor == timer.childId) {
    int requestedLevel = getValidPercentage(message);
    long value = (int)(requestedLevel / 100. * TIMER_MAX_DURATION); // % of TIMER_MAX_DURATION
    
    #ifdef DEBUG_MSG
      Serial.print("Interpreted timer max duration change request (% / from / to / max): ");
      Serial.print(requestedLevel);
      Serial.print(" / ");
      Serial.print(timer.duration);
      Serial.print(" / ");
      Serial.print(value);
      Serial.print(" / ");
      Serial.print(TIMER_MAX_DURATION);
      
      Serial.println();
    #endif

    timer.duration = value;

    saveTimer();
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

MyMessage motionMsg(motion.childId,S_MOTION);
void on(boolean force) {
  if (!isOn) {
    isOn = true;
    on(&rgb.red, force);
    on(&rgb.green, force);
    on(&rgb.blue, force);

    send(motionMsg.set(1));
  }
}

void off() {
  if (isOn) {
    off(&rgb.red);
    off(&rgb.green);
    off(&rgb.blue);
    isOn = false;

    send(motionMsg.set(0));
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

