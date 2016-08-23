#ifndef Config_h
#define Config_h

#include <DallasTemperature.h>
#include <OneWire.h>

//#define DEBUG_PROCESS
//#define DEBUG_EEPROM
//#define DEBUG_COLOR
//#define DEBUG_MSG
//#define DEBUG_MOTION
//#define DEBUG_PHOTOCELL
//#define DEBUG_TEMPERATURE

//#define DEV_MODE

const int PHOTOCELL_MAX_RANGE = 2000;
const int TIMER_MAX_DURATION = 300; // 5 mins
const byte TEMPERATURE_MIN_DELTA = 1; // send new temp on if diff of 1 deg with last measurement
const int TEMPERATURE_READ_TIMEOUT = 5000; // read temp only each 5000 ms

/*
 * GATEWAY 
 */
#ifndef DEV_MODE
// prod
#define SN "MotionDimmableLEDStrip"
#define SV "1.2"
#define NODE_ID 10
#endif

#ifdef DEV_MODE
// dev
#define SN "MotionDimmableLEDStrip_DEV"
#define SV "1.2"
#define NODE_ID 20
#endif

MySensor gateway;

/*
 * LED
 */

COLOR red = {
  103,
  5,
  0
};

COLOR green = {
  105,
  6,
  0
};

COLOR blue = {
  106,
  7,
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

/*
 * PHOTOCELL SENSOR
 */

PHOTOCELL photocell = {
  120,     // childId
  0,       // pin: A0
  0,       // current: current read value
  2000     // level: max allowed level
};

TIMER timer = {
  121,     // childId
  300    // default duration: in s
};


TEMPERATURE temp = {
  122,  // childId
  3,    // pin: A3
  -128. // last value
};

MyMessage temperatureMsg(temp.childId,V_TEMP);


#endif
