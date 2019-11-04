#include "header.h"

/*Custom data structure to replace the default arduino servo
  library with a more bare-metal, more efficient deployment
  that is more light in with the EPROM and enhances longevity*/
struct servoData {
  bool enabled;
  String fName;
  byte potPin;
  byte serPin;
  int minP;
  int maxP;
  int minS;
  int maxS;
  byte potReading;
  byte prevReading;
  byte prevReading2;
};
