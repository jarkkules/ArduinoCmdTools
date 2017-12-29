#ifndef CmdESP_h
#define CmdESP_h

#define MAX_MSG_SIZE 140

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define USE_MYSENSORS

//#ifndef DEBUG_PRINT
//#define DEBUG_PRINT 
//#endif

#ifndef ONEWIRE_CRC
#define ONEWIRE_CRC 1
#endif

#ifndef ONEWIRE_CRCDOWNSHIFT127
#define ONEWIRE_CRCDOWNSHIFT127 1
#endif
#include <inttypes.h>
//#include <avr/pgmspace.h>
//#include "HardwareSerial.h"

// command line structure
typedef struct _cmd_t
{
    char *cmd;
    void (*func)(int argc, char **argv);
    struct _cmd_t *next;
} cmd_t;

class CmdESP
{
  private:
	Stream* _serialIn;
    Stream* _serialOut;
    char msgIn[MAX_MSG_SIZE+1]; //buffer for listen method
	
	char *msg_ptr;
	bool crcCheck;
	byte msgInCount;
	byte msgOutCount;
	
	cmd_t *cmd_tbl_list, *cmd_tbl, *cmd_entryDefault;

	void cmdStructParse(char *cmd);
	#ifdef USE_MYSENSORS
		void cmdMySensorsParse(char *cmd);	
		cmd_t *cmd_mySensors;
		uint8_t msgIndex;
		uint8_t semiColCount; 
	#endif
	//void cmdParse(char* msg);
	

  public:
	//char *defaulCommand;
	CmdESP(Stream &serialIn, Stream &serialOut);
	void cmdAdd(char *name, void (*func)(int argc, char **argv));
	void cmdAdd(char *name, void (*func)(int argc, char **argv), uint8_t defaultCmd);
	void cmdPoll();
	void cmdHandler(char ch);
	void cmdSend(char *cmd, bool addCRC8);
	uint8_t crc8(char *addr, uint8_t len);

};

#endif


