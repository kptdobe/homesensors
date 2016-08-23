#ifndef Config_h
#define Config_h

#include <DallasTemperature.h>
#include <OneWire.h>

#define DEBUG_PROCESS
#define DEBUG_EEPROM
#define DEBUG_MSG
//#define DEBUG_MOTION
//#define DEBUG_PHOTOCELL
//#define DEBUG_TEMPERATURE

//#define DEV_MODE

const byte RELAY_OFF = 0;
const byte RELAY_ON = 1;

/*
 * GATEWAY 
 */
#ifndef DEV_MODE
// prod
#define SN "Pool"
#define SV "1.1"
#define NODE_ID 11
#endif

#ifdef DEV_MODE
// dev
#define SN "Pool_Dev"
#define SV "1.1"
#define NODE_ID 21
#endif

MySensor gateway;

RELAY poolLight = {
  103,
  3,
  RELAY_OFF
};

#endif
