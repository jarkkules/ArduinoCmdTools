/*
Copyright (c) 2015


//--------------------------------------------------------------------------
*/

#include "CmdESP.h"


CmdESP::CmdESP(Stream &serialIn, Stream &serialOut)
{
	_serialIn = &serialIn;
    _serialOut = &serialOut;
	
	msg_ptr = msgIn;
	msgInCount = 0;
	
    // init the command table
    cmd_tbl_list = NULL;
	//default command entry
	cmd_entryDefault = NULL;
	#ifdef USE_MYSENSORS
		cmd_mySensors = NULL;
		semiColCount = 0;
		msgIndex = 0;
	#endif
}

void CmdESP::cmdPoll(){
	
	while (_serialIn->available()) {
	    cmdHandler(_serialIn->read());
    }

}

void CmdESP::cmdHandler(char ch){

    //char c = _serialIn->read();
	
    switch (ch)
    {
	case '$': //Without CRC check
        // msgIn[0] = '\0';
		msg_ptr = msgIn;
		msgInCount = 0;
		crcCheck =  false;
		#ifdef USE_MYSENSORS
		msgIndex = 0;
		semiColCount = 0;		
		#endif
#ifdef DEBUG_PRINT		
		_serialOut->print("noCRC\r\n");
#endif
        break;
	case '^': //With CRC check
        // msgIn[0] = '\0';
		msg_ptr = msgIn;
		msgInCount = 0;
		crcCheck =  true;
#ifdef DEBUG_PRINT		
		//char bufOut[] = "CRC\r\n";
		_serialOut->print("CRC\r\n");
#endif
        break;
#ifdef USE_MYSENSORS
	case ';':
		
		if(msgInCount++ < MAX_MSG_SIZE)
		{
			if (semiColCount < 5)
			{
				semiColCount++;
				if(semiColCount == 5)
				{
					msgIndex = msgInCount;
					*msg_ptr++ = ch;
				}
				else
				{
					*msg_ptr++ = '/';
				}
			}
			else
			{
				*msg_ptr++ = ch;
			}
		}
		break;
#endif
	case '\n':
	case '\r':
		if ((crcCheck && msgInCount < 3) || (!crcCheck && msgInCount == 0) )
		{
			msg_ptr = msgIn;
			msgInCount = 0; 
			crcCheck =  false;
			#ifdef USE_MYSENSORS
			msgIndex = 0;
			semiColCount = 0;		
			#endif			
#ifdef DEBUG_PRINT		
			_serialOut->println("ERR3");
#endif
			return;
		}

	    // terminate the msg and reset the msg ptr. then send
        // it to the handler for processing.
        *msg_ptr = '\0';
		
		if(crcCheck) {
			msgInCount--;
			uint8_t crc = crc8((char *)msgIn, msgInCount);
			if(msgIn[msgInCount] != crc) {
				msg_ptr = msgIn;
				msgInCount = 0;
				_serialOut->println("ERR2");
				_serialOut->print(msgInCount);
#ifdef DEBUG_PRINT		
				_serialOut->print("-size\r\n");
				_serialOut->print(crc, DEC);
				_serialOut->print("-crc DEC\r\n");
				_serialOut->print(crc, HEX);
				_serialOut->print("-crc HEX\r\n");
#endif
				return;
			}

			*msg_ptr--;
			*msg_ptr = '\0';
		}
		#ifdef USE_MYSENSORS
			if (semiColCount == 5)
			{
				cmdMySensorsParse((char *)msgIn);
			}
			else
			{
				cmdStructParse((char *)msgIn);
			}
		#else
			cmdStructParse((char *)msgIn);
		#endif
        msg_ptr = msgIn;
		msgInCount = 0;
		#ifdef USE_MYSENSORS
		msgIndex = 0;
		semiColCount = 0;		
		#endif
        break;

    default:
        // normal character entered. add it to the buffer
		if (msgInCount++ < MAX_MSG_SIZE) {
			*msg_ptr++ = ch;
#ifdef DEBUG_PRINT		
			_serialOut -> print(ch);
#endif
		}
		else
		{
			_serialOut -> print("ERR0\r\n");
		}
        break;
    }
}

//parse command and parameters from string
void CmdESP::cmdStructParse(char *cmd)
{
#ifdef DEBUG_PRINT		
	_serialOut->print("Parse:");
	_serialOut->println(cmd);
#endif
	uint8_t argc, i = 0;
    char *argv[30];
    char buf[50];
    cmd_t *cmd_entry;
	

    //fflush(stdout);

    // parse the command line statement and break it up into space-delimited
    // strings. the array of strings will be saved in the argv array.
    argv[i] = strtok(cmd, " ");
    do
    {
        argv[++i] = strtok(NULL, " ");
    } while ((i < 30) && (argv[i] != NULL));
    
    // save off the number of arguments for the particular command.
    argc = i;
	
    // parse the command table for valid command. used argv[0] which is the
    // actual command name typed in at the prompt
    for (cmd_entry = cmd_tbl; cmd_entry != NULL; cmd_entry = cmd_entry->next)
    {
		if (!strcmp(argv[0], cmd_entry->cmd))
        {
            cmd_entry->func(argc, argv);
            //cmd_display();
            return;
        }
    }
	//Run default command
	if (cmd_entryDefault != NULL)
	{
		cmd_entryDefault->func(argc, argv);
		return;
	}

    // command not recognized. print message and re-generate prompt.
	char bufOut[] = "ERR1\r\n";
    _serialOut -> print(bufOut);
 }
 
 #ifdef USE_MYSENSORS
 void CmdESP::cmdMySensorsParse(char *cmd)
{
#ifdef DEBUG_PRINT		
	_serialOut->print("MySens Parse:");
	_serialOut->println(cmd);
#endif
	char *argv[2];
	argv[3] = cmd;

	if (cmd_mySensors != NULL)
	{
		uint8_t i = 0;
		argv[0] = strtok(cmd, ";");
		argv[1] = strtok(NULL, "\0");
		#ifdef DEBUG_PRINT		
			_serialOut->println(argv[0]);
			_serialOut->println(argv[1]);
		#endif		
		cmd_mySensors->func(2, argv);
		return;
	}
	else
	{
		cmdStructParse(cmd);
	}
}
#endif

 void CmdESP::cmdAdd(char *name, void (*func)(int argc, char **argv))
{
	cmdAdd(name, func, 0); 
}
 
void CmdESP::cmdAdd(char *name, void (*func)(int argc, char **argv), uint8_t cmdType)
{
    // alloc memory for command struct
    cmd_tbl = (cmd_t *)malloc(sizeof(cmd_t));

    // alloc memory for command name
    char *cmd_name = (char *)malloc(strlen(name)+1);

    // copy command name
    strcpy(cmd_name, name);

    // terminate the command name
    cmd_name[strlen(name)] = '\0';

    // fill out structure
    cmd_tbl->cmd = cmd_name;
    cmd_tbl->func = func;
    cmd_tbl->next = cmd_tbl_list;
    cmd_tbl_list = cmd_tbl;
	
	if (cmdType == 1)
	{
		cmd_entryDefault = cmd_tbl;
	}
	#ifdef USE_MYSENSORS
	else if(cmdType == 2)
	{
		cmd_mySensors = cmd_tbl;
	}
	#endif
}

//
// Send command and add optional CRC8 checksum.
//
void CmdESP::cmdSend(char *cmd, bool addCRC8)
{

#ifdef DEBUG_PRINT		
	_serialOut->print("Send:");
	_serialOut->println(cmd);
#endif
	
	_serialOut->print(addCRC8 ? '^' : '@');
	uint8_t i = 0;
	do
	{
		_serialOut->print(cmd[i]);
		i++;
	} while ((i < MAX_MSG_SIZE) && (cmd[i] != NULL));

	if (addCRC8)
	{
		uint8_t crc = crc8(cmd, i);
		_serialOut->print(char(crc));
	}
	_serialOut->print('\r');
}

//
// Compute a Dallas Semiconductor 8 bit CRC directly.
// this is much slower, but much smaller, than the lookup table.
//
uint8_t CmdESP::crc8(char *addr, uint8_t len)
{
	
//#ifdef DEBUG_PRINT		
//	_serialOut->print("CRC8:");
//	_serialOut->print(addr);
//	_serialOut->print(",");
//	_serialOut->println(len);
//#endif

	uint8_t crc = 0;
	
	while (len--) {
		uint8_t inbyte = *addr++;
		for (uint8_t i = 8; i; i--) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8C;
			inbyte >>= 1;
		}
	}

#ifdef ONEWIRE_CRCDOWNSHIFT127
	if (crc > 127)
		crc -= 127;
#endif

	return crc;
}





