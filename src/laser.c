#include <string.h>
//#include <ctype.h>

#include "common.h"
#include "main.h"

#include "HD44780.h"
#include "radio.h"


volatile uint8_t isLaserDetected = 0;

void setLaserDetection(int onoff) 
{
  isLaserDetected = onoff;
}

void laser_init(void *data)
{

}

void lcdPrintLaser(const char* str)
{
  static char prev[18]={0,};
  if (strcmp(prev, str) !=0) {
    snprintf(prev, sizeof(prev), "%s", str);
    HD44780_GotoXY(0, 1); // Move cursor to First Line First Position.
    HD44780_PutStr((char*)str);   // Now write it actually to LCD.
		HAL_Delay(1000);
  }
}

void lcdClearLaser()
{
  const char *empty = "                ";
  lcdPrintLaser(empty);
}

void laser_run(void *data)
{
  static uint8_t sequence = 0;
  char stringBuffer[16] = {0,};

  if (isLaserDetected) {
    lcdPrintLaser("Laser Detected");

    /*
     insert your code for transmitting data using nRF module
     here.
     */
    // write the data to a temporary buffer array.
    snprintf(stringBuffer, sizeof(stringBuffer), "%s-%d", "Det", sequence);
    radio_send_data(stringBuffer, strlen(stringBuffer));

    setLaserDetection(0);

  } else {
    lcdClearLaser();
  }

  /* receive data from nRF */
  if (radio_recv_data(stringBuffer, sizeof(stringBuffer)) > 0) {
	  LOG(1, "rx=(%s)", stringBuffer);
    lcdPrintLaser("Dont Point there");
  } else {
    lcdClearLaser();
  }
}

ADD_TASK(laser_run, laser_init, NULL, 0, "laser task");
