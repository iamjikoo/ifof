#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "common.h"

/* This include will give us the CubeMX generated defines */
//#include "main.h"

char log_level = 0; /* set trace level */

uint8_t getLogLevel()
{
	return log_level;
}

uint8_t setLogLevel(int level)
{
	log_level = level;

	return log_level;
}


ParserReturnVal_t CmdLog(int mode)
{
  uint16_t val,rc;
  
  if(mode != CMD_INTERACTIVE) return CmdReturnOk;

  rc = fetch_uint16_arg(&val);
  if(rc) {
    printf("Please supply 0 ~ 3 to set log level\n");
    printf("Current level: %d\r\n", log_level);
    return CmdReturnBadParameter1;
  }

	if (val < 0 || val > 3) {
    printf("Invalid log level: set level 0 to 3\r\n");
    return CmdReturnBadParameter1;
	}

	if (val != log_level) {
    printf("Log level: %d -> %d\r\n", log_level, val);
	  log_level = val;
	} else {
    printf("Log level: %d\r\n", log_level);
	}

  return CmdReturnOk;
}

ADD_CMD("log",CmdLog,"0 ~ 3           Set log level")


#if 0
ParserReturnVal_t CmdPinout(int mode)
{
  uint16_t val,rc;
  
  if(mode != CMD_INTERACTIVE) return CmdReturnOk;

  rc = fetch_uint16_arg(&val);
  if(rc) {
    printf("Please supply 0 ~ 3 to set trace level\n");
    printf("Current level: %d\r\n", trc_level);
    return CmdReturnBadParameter1;
  }

	if (val < 0 || val > 3) {
    printf("Invalid trace level: set level 0 to 3\r\n");
    return CmdReturnBadParameter1;
	}

	if (val != trc_level) {
    printf("Trace level: %d -> %d\r\n", trc_level, val);
	  trc_level = val;
	} else {
    printf("Trace level: %d\r\n", trc_level);
	}

  return CmdReturnOk;
}
#endif

#if 1 
void hexDump(const char *desc, const void* data, int len)
{
	char tmp[8];
	char hexstr[128];
	const char *ptr = (const char *)data;

	if (desc) 
		LOG(3, "%s:", desc);

	if (len <= 0) 
		return;

	for (int i = 0; i < len; i++) {
	  sprintf(tmp, "%02x ", ptr[i]);
	  if (i%16==0) {
			if (i) {
				LOG(3, "%s", hexstr);
			}
			sprintf(hexstr, " %04x %s", i, tmp);
		} else {
		   strcat(hexstr, tmp);
		}

		if (i == len-1) {
				LOG(3, "%s", hexstr);
		}
	}

}
#endif

	
