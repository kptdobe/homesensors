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
  int pin;
  int level;
  int last;
};

#endif

