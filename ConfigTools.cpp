#include <ConfigTools.h>

void ChangeSerialSettings(char *settings, long &baudrate, byte &serialConfig)
{
    char baudrateConfig[7];
    
    byte i = 0;
    while (settings[i] != ',' && i < 6 )
    {
      baudrateConfig[i] = settings[i];
      i++;
    }
    baudrateConfig[i] = '\0';
    // Serial.println(baudrateConfig);
 
    if (i < 7)
    {  
      i++;
      
      String baudString(baudrateConfig);
      baudrate = (long)baudString.toInt();

      byte parity = 0;
      switch (settings[i+1]) 
      {
        case 'N':
          parity = 0x00;
          break;
        case 'E':
          parity = 0x20;
          break;
        case 'O':
          parity = 0x30;
          break;
      }
  
      byte stopbits = 0;
      if(settings[i+2] == '1')
        stopbits = 0x00;
      else if(settings[i+2] == '2')
        stopbits = 0x08;
  
      byte databits = (settings[i] - '5') * 0x02;
    
      serialConfig = (databits+parity+stopbits);
      
      //Serial.println(baudrate);
      //Serial.println(serialConfig, HEX);
    
	} 
}

boolean IsTimeout(unsigned long startTime, unsigned long timeoutTime)
{
   unsigned long now = millis();
    if (startTime <= now)
    {
      if ( (unsigned long)(now - startTime )  < timeoutTime ) 
        return false;
    }
    else
    {
      if ( (unsigned long)(startTime - now) < timeoutTime ) 
        return false;
    }
  
    return true;
}