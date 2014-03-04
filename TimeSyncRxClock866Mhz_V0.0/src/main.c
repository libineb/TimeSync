/****************************************************************************
 * main.c
 *
 *  Created on: Jan 19, 2014
 *      Author: libin.eb
 *
******************************************************************************/
/* User defined includes*/
#include "TS_RF_Transmitt_Receive.h"
#include "TS_Timer_Pwms.h"
#include "TS_Hardware_Init.h"
#include "TS_Proximity_Sensor.h"
#include "TS_PinConfiguration.h"
#include "TS_RTC.h"

/******************************************************************************
* Outline 		: main program based on CC430F5137
* Description 	: Main Program for generating the clock pulses and receive
* 				  GPS data from the other module. Low Power mode is LPM3.
* 				  RF center can be 866Mhz, 868Mhz or 915Mhz.
* Argument  	: none
* Return value  : none
******************************************************************************/
void main(void)
{
	/* WDTPW = WDT password. Always read as 069h. Must be written as 5Ah;
		if any other value is written, a PUC is generated. WDTHOLD =  WDT Stopped
		After a PUC, the WDT_A module is automatically configured in the watchdog mode with an
		initial ~32-ms reset interval using the SMCLK */
 	WDTCTL = (WDTPW + WDTHOLD);

	/* Hardware Initialization and clock setting */
	TS_HardwareInit();

//	TS_RadioReception();						// for check

	/* All the process are handled in this function */
	TS_GPSDataHandler();

	/* While loop trap */
	while(1);
}








