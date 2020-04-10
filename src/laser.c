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
	char stringBuffer[16] = { 0 };

  snprintf(stringBuffer, 16, "Pulse:");// write the data to a temporary buffer array.
  HD44780_GotoXY(0, 0);     // Move cursor to First Line First Position.
  HD44780_PutStr(stringBuffer);       // Now write it actually to LCD.

  snprintf(stringBuffer, 16, "Laser:");// write the data to a temporary buffer array.
  HD44780_GotoXY(0, 1);     // Move cursor to First Line First Position.
  HD44780_PutStr(stringBuffer);       // Now write it actually to LCD.

}

void lcdPrintPulse(const char* str)
{
	static char prev[16]={0,};

	if (strcmp(prev, str) !=0) {
    HD44780_GotoXY(7, 0); // Move cursor to First Line First Position.
    HD44780_PutStr((char*)str);   // Now write it actually to LCD.
		snprintf(prev, sizeof(prev), "%s", str);
		printf("pulse\r\n");
	}
}

void lcdPrintLaser(const char* str)
{
	static char prev[16]={0,};
	if (strcmp(prev, str) !=0) {
    HD44780_GotoXY(7, 1); // Move cursor to First Line First Position.
    HD44780_PutStr((char*)str);   // Now write it actually to LCD.
		snprintf(prev, sizeof(prev), "%s", str);
		printf("laser\r\n");
	}
}

void lcdClearLaser()
{
	const char *empty="         ";
	lcdPrintLaser(empty);
}

void lcdClearPulse()
{
	const char *empty="         ";
	lcdPrintPulse(empty);
}

void laser_run(void *data)
{
	static uint8_t sequence = 0;
	char stringBuffer[16] = {0,};

	if (isLaserDetected) {
		// write the data to a temporary buffer array.
		snprintf(stringBuffer, sizeof(stringBuffer), "%s-%d", "Det", sequence);
		lcdPrintLaser(stringBuffer);

		/*
		 insert your code for transmitting data using nRF module
		 here.
		 */
		radio_send_data(stringBuffer, strlen(stringBuffer));

    setLaserDetection(0);

	} else {
		lcdClearLaser();
	}

	/* receive data from nRF */
	int len = radio_recv_data(stringBuffer, sizeof(stringBuffer));
	if (len) {
		printf("rx=(%s)\r\n", stringBuffer);
		lcdPrintPulse(stringBuffer);
	} else {
		lcdClearPulse();
	}
}

ADD_TASK(laser_run, laser_init, NULL, 100, "laser task");
