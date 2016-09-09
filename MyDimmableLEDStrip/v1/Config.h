#ifndef Config_h
#define Config_h

#define DEBUG_PROCESS
#define DEBUG_EEPROM
#define DEBUG_COLOR
#define DEBUG_MSG
//#define DEBUG_MOTION
//#define DEBUG_PHOTOCELL

const int PHOTOCELL_MAX_RANGE = 1000;
const int TIMER_MAX_DURATION = 20000; //300000; // 5 mins

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
  120,     // childId
  0,       // pin: A0
  0,       // current: current read value
  500      // level: max allowed level
};

TIMER timer = {
  121,     // childId
  4000    // default duration: in ms
};

#endif
