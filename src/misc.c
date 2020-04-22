#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"

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
  char *para, *n; 

  char *helpString = 
    "\r\n"
    "log  [0|1|2|3]   - Set log level\r\n\r\n";

  if (mode == CMD_SHORT_HELP) {
    return CmdReturnOk;
  }

  if (mode ==CMD_LONG_HELP) {
    rc = fetch_string_arg(&n);
    if (rc)
      printf("%s", helpString);
    return CmdReturnOk;
  }

  rc = fetch_string_arg(&para);
  if (rc) {
    printf("Current log level: %d\r\n", log_level);
    printf("%s", helpString);
    return CmdReturnOk;
  } else if (strlen(para) > 1 || para[0] < '0' || para[0] > '3') { 
    /*  type 'crypto help' or 'crypto ?' */
    printf("%s", helpString);
    return CmdReturnOk;
  }

  val = atoi(para);

  if (val != log_level) {
    printf("Log level: %d -> %d\r\n", log_level, val);
    log_level = val;
  } else {
    printf("Log level: %d\r\n", log_level);
  }

  return CmdReturnOk;
}

ADD_CMD("log",CmdLog,"0 ~ 3           Set log level")


ParserReturnVal_t CmdPinout(int mode)
{
  if(mode != CMD_INTERACTIVE) 
    return CmdReturnOk;

  char *pinOutShape = 
    "\r\n"
    "              +--[mUSB]--+\r\n"
    " LCD (D7) PA9 |          | VIN\r\n"
    " LCD (D6) PA10|          | GND\r\n"
    "          NRST|  Nucleo  |NRST\r\n"
    "          GND |  L432KC  | +5V\r\n"
    "          PA12|          | PA2 USART2 Tx\r\n"
    " LCD (RS) PB0 |          | PA7 nRF24 (SPI1_MOSI)\r\n"
    "          PB7 |          | PA6 NRF24 (SPI1_MISO)\r\n"
    "          PB6 |          | PA5 nRF24 (SPI1_SCK)\r\n"
    " LCD (EN) PB1 |          | PA4 nRF24 (CE)\r\n"
    "          NC  |          | PA3 nRF24 (CSN)\r\n"
    "          NC  |          | PA1 Pulse Sensor\r\n"
    " Laser Tx PA8 |          | PA0 \r\n"
    " Laser Rx PA11|          |AREF\r\n"
    " LCD (D4) PB5 |          |+3V3 --> nRF24L01 Power\r\n"
    " LCD (D5) PB4 |          | PB3 LCD (RW)\r\n"
    "              +----------+\r\n";

  printf("%s", pinOutShape);

  return CmdReturnOk;
}
ADD_CMD("pinout",CmdPinout,"                Show pinout")

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
