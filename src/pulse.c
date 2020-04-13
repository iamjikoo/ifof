/* pulse sensor */

#include <string.h>

#include "common.h"
#include "main.h"

#include "pulse.h"

#if 0
#include "interrupt_handler.h"
#include "interrupt_config.h"
#include "bsp_led.h"
#include "bsp_usart1.h"
#include "bsp_adc.h"
#endif

//__IO uint32_t Signal;
#define NUM_OF_SAMPLES 4096
extern uint16_t sampleMeasured[NUM_OF_SAMPLES];

__IO int rate[10] = {0,};             // array to hold last ten IBI values
__IO unsigned long sampleCounter = 0; // used to determine pulse timing
__IO unsigned long lastBeatTime = 0;  // used to find IBI
__IO int P = 512;                     // used to find peak in pulse wave, seeded
__IO int T = 512;                     // used to find trough in pulse wave, seeded
__IO int thresh = 512;                // used to find instant moment of heart beat, seeded
__IO int amp = 100;                   // used to hold amplitude of pulse waveform, seeded
__IO int firstBeat = TRUE;            // used to seed rate array so we startup with reasonable BPM
__IO int secondBeat = FALSE;          // used to seed rate array so we startup with reasonable BPM
__IO int BPM;                         // used to hold the pulse rate
__IO int IBI = 600;                   // holds the time between beats, must be seeded! 
__IO int Pulse = FALSE;               // true when pulse wave is high, false when it's low
__IO int QS = FALSE;                  // becomes true when Arduoino finds a beat.
  
__IO int      counter = 9;            // count timer interrupt to ten to send data
__IO uint32_t preSignal = 500;        // store the last signal value
__IO int      Status_Flag = ABNORMAL; // status flag stands for the state of current heart wave


void pulse_process(uint32_t tempSignal);

int detect_trigger(uint32_t tempSignal);
int detect_state();
void calculate_process(uint32_t tempSignal);
void normal_process(uint32_t tempSignal);
void abnormal_process(uint32_t tempSignal);

void Delay(__IO uint32_t nCount)
{
  while(nCount--);
}

void sendDataToProcessing(char symbol, int data)
{
  printf("%c%d\n", symbol, data);
}

void pulse_process(uint32_t tempSignal)
{

  // detect if the current signal is a trigger signal
  if(detect_trigger(tempSignal))                            
  {// detected a trigger, change current state
    if(Status_Flag == NORMAL)
    {
      abnormal_process(tempSignal);
      Status_Flag = ABNORMAL;
    }
    else
    {
      normal_process(tempSignal);
      Status_Flag = NORMAL;
    }
  }
  else
  {// maintain the current state
    if(Status_Flag == ABNORMAL)
      abnormal_process(tempSignal);
    else
      normal_process(tempSignal);
  }
}

/* used to detect trigger TRUE stands for needing to change state, FLASE not */
int detect_trigger(uint32_t tempSignal)
{
	int changeFlag = FALSE;                 // the first detect flag
	int abnormalFlag = FALSE;               // the return value of detect_state() method
	int flag = FALSE;                       // return value
	
	if(Status_Flag == NORMAL && BPM > 180)  // BPM>180 stands for abnormal so change current state
	{
		return TRUE;
	}
	
	// detect the signal value, delay a little bit time detect again to confirm if it's a trigger signal
	if(tempSignal <= LOWER_TRIGGER && (tempSignal - preSignal) < -TRIGGER_DELTA )
	{
		Delay(0xff);
		//if((Signal>>2) <= LOWER_TRIGGER)
		if(tempSignal <= LOWER_TRIGGER)
		{
			changeFlag = 1;
			abnormalFlag = detect_state();      // detect if the following heart wave is normal or not
		}
	}
	else if(tempSignal > UPPER_TRIGGER && (tempSignal - preSignal) > TRIGGER_DELTA )
	{
		Delay(0xff);
		//if((Signal>>2) > UPPER_TRIGGER)
		if(tempSignal > UPPER_TRIGGER)
		{
			changeFlag = 1;
			abnormalFlag = detect_state();
		}
	}
	
	if(changeFlag == FALSE)
		 flag = FALSE;
	else if(Status_Flag == NORMAL && abnormalFlag == TRUE)
		 flag = TRUE;
	else if(Status_Flag == ABNORMAL && abnormalFlag == FALSE)
		 flag = TRUE;
	
	return flag;
}

/* detect 1500 signal values to judge if the following heart wave is normal or not*/
int detect_state()
{
	int abnormalFlag = FALSE;                   // return value
	//uint32_t detectedSignal = Signal >> 2;      // current signal value
	uint16_t *pSample = sampleMeasured;
	uint16_t detectedSignal = *pSample;         // current signal value
	int abnormalCounter = 0;                    // the number of signal value between 513 and 520
	int sumCounter = 0;                         // control the num of detection, which equals to 1500
	uint64_t sumAll = 0;                        // sum of signal value
	uint64_t sumSquare = 0;                     // sum of the square of signal
	uint64_t variance = 0;                      // the variance of signal value
	uint64_t addNumber = 0;                     // the number of appropriate(400 to 700) signal value
	
	Delay(0xffff);
	// begin detection
	while(sumCounter != 1500)
	{
		if(detectedSignal < 700 && detectedSignal > 400)
		{
			sumAll += detectedSignal;               // add the detected signal value
			sumSquare += detectedSignal * detectedSignal;// add the square of detected signal value
			++ addNumber;                           // add number
		}
		++ sumCounter;

		// during the detection maintain previous state
		if(Status_Flag == ABNORMAL)
		{
			abnormal_process(detectedSignal);
			if(detectedSignal > 513 && detectedSignal < 520)
				++ abnormalCounter;                   // if the signal value is in (513, 520), add number
		}
		if(Status_Flag == NORMAL)
		{
			normal_process(detectedSignal);
			if(detectedSignal > 513 && detectedSignal < 520)
				++ abnormalCounter;
		}
		Delay(0xfff);                             // delay to get new signal value
		//detectedSignal = Signal >> 2;
		detectedSignal = *(++pSample);
	}
	
	// calculate variance
	sumAll /= addNumber; 
	sumSquare /= addNumber;
	variance = sumSquare - sumAll * sumAll;
	
	// if there are a lot of signal value between 513 and 520, or the variance is small enough
	// consider it as a abnormal state
	if(abnormalCounter > 250 || variance < 1000)
		abnormalFlag = TRUE;
	
	return abnormalFlag;
}

/* main method used to process signal */
void calculate_process(uint32_t tempSignal)
{
	int interval = 0;
	int i = 0;
	int runningTotal = 0;

	sampleCounter += 2;                             // keep track of the time in mS with this variable
  interval = sampleCounter - lastBeatTime;        // monitor the time since the last beat to avoid noise

  // find the peak and trough of the pulse wave
  if(tempSignal < thresh && interval > (IBI/5)*3){// avoid dichrotic noise by waiting 3/5 of last IBI
    if (tempSignal < T){                          // T is the trough
      T = tempSignal;                             // keep track of lowest point in pulse wave 
    }
  }

  if(tempSignal > thresh && tempSignal > P){      // thresh condition helps avoid noise
    P = tempSignal;                               // P is the peak
  }                                               // keep track of highest point in pulse wave

  // NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // tempsignal surges up in value every time there is a pulse
  if (interval > 250){                            // avoid high frequency noise
		if ((tempSignal > thresh) && (Pulse == FALSE) && (interval > (IBI/5)*3) ){        
      Pulse = TRUE;                               // set the Pulse flag when we think there is a pulse
			//LED_On();                                   // turn on LED
      IBI = sampleCounter - lastBeatTime;         // measure time between beats in mS
      lastBeatTime = sampleCounter;               // keep track of time for next pulse

      if(secondBeat){                             // if this is the second beat, if secondBeat == True
        secondBeat = FALSE;                       // clear secondBeat flag
        for(i=0; i<=9; i++){                      // seed the running total to get a realisitic BPM at startup
          rate[i] = IBI;                      
        }
      }

      if(firstBeat){                              // if it's the first time we found a beat, if firstBeat == True
        firstBeat = FALSE;                        // clear firstBeat flag
        secondBeat = TRUE;                        // set the second beat flag
				return;                                   // IBI value is unreliable so discard it
      }
			
      // keep a running total of the last 10 IBI values
      for(i=0; i<=8; i++){                        // shift data in the rate array
        rate[i] = rate[i+1];                      // and drop the oldest IBI value 
        runningTotal += rate[i];                  // add up the 9 oldest IBI values
      }

      rate[9] = IBI;                              // add the latest IBI to the rate array
      runningTotal += rate[9];                    // add the latest IBI to runningTotal
      runningTotal /= 10;                         // average the last 10 IBI values 
      BPM = 60000/runningTotal;                   // how many beats can fit into a minute? that's BPM!
      QS = TRUE;                                  // set Quantified Self flag 
      // QS FLAG IS NOT CLEARED INSIDE THIS ISR
    }                       
  }

  if (tempSignal < thresh && Pulse == TRUE){      // when the values are going down, the beat is over
    //LED_Off();                                    // turn off LED
    Pulse = FALSE;                                // reset the Pulse flag so we can do it again
    amp = P - T;                                  // get amplitude of the pulse wave
    thresh = amp/2 + T;                           // set thresh at 50% of the amplitude
    P = thresh;                                   // reset these for next time
    T = thresh;
  }

  if (interval > 2500){                           // if 2.5 seconds go by without a beat
    thresh = 512;                                 // set thresh default
    P = 512;                                      // set P default
    T = 512;                                      // set T default
    lastBeatTime = sampleCounter;                 // bring the lastBeatTime up to date        
    firstBeat = TRUE;                             // set these to avoid noise
    secondBeat = FALSE;                           // when we get the heartbeat back
  }
	//ledFadeToBeat();
}

/* used to process the heart wave in normal state */
void normal_process(uint32_t tempSignal)
{
	calculate_process(tempSignal);
	--counter;
	// count form 9 to 0 every 20ms to send
	if(counter == 0)
	{
		if(tempSignal > 750 || tempSignal < 400)       // used to control wave
			sendDataToProcessing('S',512);
		else
			sendDataToProcessing('S', tempSignal);
		if (QS == TRUE)
		{// Quantified Self flag is True when finds a heartbeat
			//fadeRate = 4000;	
			sendDataToProcessing('B',BPM);               // send heart rate with a 'B' prefix
			sendDataToProcessing('Q',IBI);               // send time between beats with a 'Q' prefix
			QS = FALSE;                                  // reset the Quantified Self flag for next time    		
		}
		counter = 9;
	}
	preSignal = tempSignal;                          // store current signal value
}

/* used to show straight line in abnormal state */
void abnormal_process(uint32_t tempSignal)
{
	--counter;
	if (!counter)
	{
		BPM = 0;
		IBI = 600;
		sendDataToProcessing('S', 512);
		sendDataToProcessing('B', 0);                  // send heart rate with a 'B' prefix
		sendDataToProcessing('Q', IBI);                // send time between beats with a 'Q' prefix
		counter = 9;
	}
	preSignal = tempSignal;
}

uint32_t adcVal[2];

void pulse_init(void *data)
{

	//HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcVal, 2);

}

void pulse_run(void *data)
{

}

ADD_TASK(pulse_run, pulse_init, NULL, 100, "laser task");
