#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

void ChangeSerialSettings(char *settings,  long &baudrate, byte &serialConfig);

boolean IsTimeout(unsigned long startTime, unsigned long timeoutTime);