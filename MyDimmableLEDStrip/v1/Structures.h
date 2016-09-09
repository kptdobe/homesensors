#ifndef Structures_h
#define Structures_h

struct COLOR {
  int childId;
  int pin;
  int level;
};

struct RGB {
  COLOR red;
  COLOR green;
  COLOR blue;
};

struct MOTION {
  int childId;
  int pin;
  boolean current;
  boolean last;
  boolean forced;
};

struct PHOTOCELL {
  int childId;
  int pin;
  int current;
  int maxLevel;
};

struct TIMER {
  int childId;
  int duration;
};

struct TEMPERATURE {
  int childId;
  int pin;
  float last;
};

#endif

