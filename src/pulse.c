/* pulse sensor */

#include <string.h>

#include "common.h"
#include "main.h"
#include "HD44780.h"

#define TRUE 1
#define FALSE 0

volatile uint8_t data_ready = 0;

volatile int rate[10]; // array to hold last ten IBI values
volatile unsigned long sampleCounter = 0; // used to determine pulse timing
volatile unsigned long lastBeatTime = 0; // used to find IBI

volatile int P = 2048; // used to find peak in pulse wave, seeded
volatile int T = 2048; // used to find trough in pulse wave, seeded

volatile int N = 0;

volatile int thresh = 2048; // used to find instant moment of heart beat, seeded
volatile int amp = 100; // used to hold amplitude of pulse waveform, seeded

volatile uint8_t firstBeat = TRUE; // used to seed rate array so we startup with reasonable BPM
volatile uint8_t secondBeat = FALSE; // used to seed rate array so we startup with reasonable BPM

volatile int BPM; // int that holds raw Analog in 0. updated every 2mS
volatile int Signal; // holds the incoming raw data
volatile int IBI = 750; // int that holds the time interval between beats! Must be seeded!
volatile uint8_t Pulse = FALSE; // "True" when User's live heartbeat is detected. "False" when not a "live beat".
volatile uint8_t QS = FALSE; // becomes true when finds a beat.
uint16_t adcAvgValue = 0;


void calculate_heart_beat(uint16_t val) {

  Signal = val;

  sampleCounter += 2; // keep track of the time in mS with this variable
  N = sampleCounter - lastBeatTime; // monitor the time since the last beat to avoid noise

  //  find the peak and trough of the pulse wave
  if (Signal < thresh && N > (IBI / 5)*3) {
    // avoid dichrotic noise by waiting 3/5 of last IBI
    if (Signal < T) {
      // T is the trough
      T = Signal;
      // keep track of lowest point in pulse wave
    }
  }

  if (Signal > thresh && Signal > P) { // thresh condition helps avoid noise
    P = Signal; // P is the peak
  } // keep track of highest point in pulse wave

  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // signal surges up in value every time there is a pulse
  if (N > 250) { // avoid high frequency noise
    if ((Signal > thresh) && (Pulse == FALSE) && (N > (IBI / 5)*3)) {
      Pulse = TRUE; // set the Pulse flag when we think there is a pulse
      IBI = sampleCounter - lastBeatTime; // measure time between beats in mS
      lastBeatTime = sampleCounter; // keep track of time for next pulse

      if (secondBeat) { // if this is the second beat, if secondBeat == TRUE
        secondBeat = FALSE; // clear secondBeat flag
        int i;
        for (i = 0; i <= 9; i++) { // seed the running total to get a realisitic BPM at startup
          rate[i] = IBI;
        }
      }

      if (firstBeat) { // if it's the first time we found a beat, if firstBeat == TRUE
        firstBeat = FALSE; // clear firstBeat flag
        secondBeat = TRUE; // set the second beat flag

        return; // IBI value is unreliable so discard it
      }

      // keep a running total of the last 10 IBI values
      uint16_t runningTotal = 0; // clear the runningTotal variable
      int i;
      for (i = 0; i <= 8; i++) { // shift data in the rate array
        rate[i] = rate[i + 1]; // and drop the oldest IBI value
        runningTotal += rate[i]; // add up the 9 oldest IBI values
      }

      rate[9] = IBI; // add the latest IBI to the rate array
      runningTotal += rate[9]; // add the latest IBI to runningTotal
      runningTotal /= 10; // average the last 10 IBI values
      BPM = 60000 / runningTotal; // how many beats can fit into a minute? that's BPM!
      QS = TRUE; // set Quantified Self flag
      // QS FLAG IS NOT CLEARED INSIDE THIS ISR
    }
  }

  if (Signal < thresh && Pulse == TRUE) { // when the values are going down, the beat is over
    Pulse = FALSE; // reset the Pulse flag so we can do it again
    amp = P - T; // get amplitude of the pulse wave
    thresh = amp / 2 + T; // set thresh at 50% of the amplitude
    P = thresh; // reset these for next time
    T = thresh;
  }

  if (N > 2500) { // if 2.5 seconds go by without a beat
    thresh = 2048;// 550; // set thresh default
    P = 2048;// 512; // set P default
    T = 2048;// 512; // set T default
    lastBeatTime = sampleCounter; // bring the lastBeatTime up to date
    firstBeat = TRUE; // set these to avoid noise
    secondBeat = FALSE; // when we get the heartbeat back
    QS = FALSE;
    BPM = 0;
    IBI = 600;                  // 600ms per beat = 100 Beats Per Minute (BPM)
    Pulse = FALSE;
    amp = 100;                  // beat amplitude 1/10 of input range.
  }
}

void lcdPrintPulse(const char* str)
{
  static char prev[18]={0,};

  if (strcmp(prev, str) !=0) {
    snprintf(prev, sizeof(prev), "%s", str);
    HD44780_GotoXY(6, 0); // Move cursor to First Line First Position.
    HD44780_PutStr((char*)str);   // Now write it actually to LCD.
    HAL_Delay(100);
  }
}

void lcdClearPulse()
{
  const char *empty = "     ";
  lcdPrintPulse(empty);
}

void pulse_run(void  *data)
{
  char stringBuffer[16] = {0,};

  if (QS == TRUE) {
    // A Heartbeat Was Found, BPM and IBI have been Determined
    // Quantified Self "QS" true when we find a heartbeat
    QS = FALSE; // reset the Quantified Self flag for next time
    printf("IBI: %d \tBPM: %d \n\r",IBI,BPM);
    snprintf(stringBuffer, sizeof(stringBuffer), " %d ", BPM);
    lcdPrintPulse(stringBuffer);
  }
}

ADD_TASK(pulse_run, NULL, NULL, 0, "pulse task");
