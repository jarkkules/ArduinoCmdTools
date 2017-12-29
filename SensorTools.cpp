#include "SensorTools.h"
//#define ANALOG_DEBUG
//Measure analog channel
int  MeasureAnalogChannel(uint8_t sensorPin, uint8_t samples)
{
  
  int sensorValue = 0;
  long sumValue = 0;
  //sensorValue = analogRead(sensorPin);
  for (uint8_t i = 0; i < samples;  i++)
  {
	delay(5);
	sensorValue = analogRead(sensorPin);
	sumValue += sensorValue;
	#ifdef ANALOG_DEBUG
      Serial.print("read");
	  Serial.print(i);
	  Serial.print(": ");
      Serial.println(sensorValue);
	#endif
  }
  return (int)sumValue/samples;

  
}
