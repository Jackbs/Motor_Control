/***************************************************************************/
/*                                                                         */
/*    A simple program to set port values and output them                  */
/*                                                                         */
/***************************************************************************/


/***  Include Files  *******************************************************/


#include "phys340libkeil.h"
#include <t89c51ac3.h>
#include <string.h>
#include <stdio.h>


int currenttime;
int lasttime;
int timedif;
double lastRPM;
double RPM;
int samples = 6;
double avgRPM[6];
double avgRPMcopy[6];
double PC,DC,IC;
double sum;

/***  Interrupt handeler  **********************************************/


void MyIntHandler(void) interrupt 6
{

  if(CCF1){ //check to make sure inturupt from module 2 was triggered
		currenttime = CCAP1H;
		currenttime = currenttime << 8;
		currenttime = currenttime + CCAP1L;
		
		if(currenttime>lasttime){
			timedif = currenttime-lasttime;
		}else{
			timedif = (currenttime+65536)-lasttime;
		}
		
		
		
		lasttime = currenttime;		 
		CCF1 = 0; //clear the interrupt flag
	}
}

float Avg(){
	int i;
	double avg = 0;;
	for(i = 0;i<samples;i++){
		avg = avg+avgRPM[i];
	}
	return (float)(avg/(double)samples);
}






/**  Main Function  ********************************************************/
void main()
{
	initLCD();
	
	CCAPM0 = 0x42; //Enable compeare mode on module 0 and set to PWM [0100 0010]
	CCAPM1 = 0x21; //Enable capture funtion on module 1 and set CCF1 bit to generate inturupt ; [00100001]
	CMOD = 0x00; //[00000000]
	EA = 1;
	EC = 1;
	CR = 1; //turn the PCA counter CCon reg
	RPM = 0.0;
	
	/**  PID CONSTANTS **/	
	PC = 0.08;
	DC = 0.02;
	IC = 0.001;
	
	sum = 0;
 	while (TRUE)
	{	
		int i;
		char outputText [31];
		float ActualRPM;
		int SetRPM;
		double difference;
		double olddifference;
		double gradient;
		
		int j = P2;
			
		lastRPM = (300000.0/timedif); //(double)((1000000.0/((double)))/200.0)*60;
		
		for(i = 0;i<(samples-1);i++){
			avgRPM[i] = avgRPM[i+1];
		}
		avgRPM[samples-1] = lastRPM;
		
		
		j = (255-j);
		SetRPM = P2;
		ActualRPM = Avg();
		
		difference = (double)((double)SetRPM-(double)ActualRPM);
		
		sum = sum+difference;
		
		gradient = difference-olddifference;
		
		olddifference = difference;
		
		//RPM = RPM + (PC*difference);
		RPM = RPM + (PC*difference)+(IC*sum)+(DC*gradient);
		
		if(RPM<0){
			RPM = 1;
		}
		if(RPM<0){
			RPM = 1;
		}
		if(SetRPM<15){ //Cannot Activate motor bellow this speed, so do not try
			RPM = 0;
		}
 		CCAP0H = (255-(int)RPM); //SET PWM
		
		
		//sprintf(outputText, "[%d]R-A[%.1f]",SetRPM, ActualRPM);
		sprintf(outputText, "[%d]R-A[%.1f]D[%.1f]R[%.1f]",SetRPM, ActualRPM,(float)sum,(float)RPM);
		
		delaya(10000);
		
		clearLCD();
		writeLineLCD(outputText);
		
		
		
		
	}	
}

