#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


//uint8_t MeasureAnalogChannel(uint8_t sensorPin, uint8_t samples);
int MeasureAnalogChannel(uint8_t sensorPin, uint8_t samples);

