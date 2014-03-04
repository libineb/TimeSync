/*
 * TS_RTC.c
 *
 *  Created on: May 9, 2013
 *      Author: libin.eb
 */

/* Header files */
#include "TS_RTC.h"

/* variables for RTC */
volatile uint8 ucRTCAlarmSetFlag = FALSE;

/******************************************************************************
* Outline 		: TS_RTC_SetAlarm
* Description 	: Program for starting the RTC Clock and generating the event
* 				  interrupt
* Argument  	: none
* Return value  : None.
******************************************************************************/
void TS_RTC_SetAlarm (void)
{
	/*Alarm will set the DoW 0 means every sunday */
	RTCAMIN  = (ALARM_SET_MASK | (ALARM_MIN_MASK  & 0x00));
	RTCAHOUR = (ALARM_SET_MASK | (ALARM_HOUR_MASK & 0x00));
	RTCADOW  = (ALARM_SET_MASK | (ALARM_DOW_MASK & 0x00));
	RTCADAY  = (ALARM_DAY_MASK & 0);						// Day alarm not used.

	RTCCTL01 &= ~(RTCAIFG + RTCTEVIFG + RTCRDYIFG);			// flags disabled.
	RTCCTL01 &= ~(RTCTEVIE);								// event alarm disabled
	RTCCTL01 |= RTCAIE;										// Alarm set
}
/******************************************************************************
* Outline 		: TS_RTC_RegisterSetStart
* Description 	: Program for starting the RTC Clock and generating the event interrupt
* Argument  	: none
* Return value  : SUCCESS or FAILURE.
******************************************************************************/
uint8 TS_RTC_RegisterSetStart(uint8 ucHour,uint8 ucMinute,uint8 ucSeconds,uint8 ucDOW)
{
	 if((ucHour >= RTC_HOUR_MAX) || (ucMinute >= RTC_MINUTE_MAX) || (ucSeconds >= RTC_SECONDS_MAX) || (ucDOW > RTC_DOW_MAX))
	 {
		 return FAILURE;
	 }

	 /* switching the RTC to counter mode to disable the calendar mode and switch back to calendar mode to
	  *  clear the all the registers */
	  RTCCTL01 &= ~(RTCMODE + RTCHOLD);
	  RTCCTL01 |= RTCMODE;

	  /* Setting registers the current UTC time */
	  RTCHOUR = ucHour;
	  RTCMIN  = ucMinute;
	  RTCSEC  = ucSeconds;
	  RTCDOW  = ucDOW;
	  RTCDAY  = 1;

	  return SUCCESS;
}

/******************************************************************************
* Outline 		: TS_RTC_Read_Stop.
* Description 	: Program for stopping the RTC Clock.
* Argument  	: none
* Return value  : None.
******************************************************************************/
void TS_RTC_ReadStop(void)
{
     RTCCTL01 &= ~RTCMODE;				// Shift from calendar mode to counter mode for
										// stopping the RTC registers and clearing those.
}
/******************************************************************************
* Outline 		: TS_RTCReturnIntFlag.
* Description 	: Program for returning the interrupt flag value.
* Argument  	: none
* Return value  : value of the alarm set flag.
******************************************************************************/
uint8 TS_RTCReturnIntFlag(void)
{
	return ucRTCAlarmSetFlag;
}
/******************************************************************************
* Outline 		: TS_RTCClearIntFlag.
* Description 	: Program for clearing the flag value.
* Argument  	: none
* Return value  : None.
******************************************************************************/
void TS_RTCClearIntFlag(void)
{
	ucRTCAlarmSetFlag = FALSE;
}

/**************************************
 Interrupt Service Routines
**************************************/
/*Interrupt Service routine for RTC*/
#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)
{
  switch(__even_in_range(RTCIV,16))
  {
    case 0 : break;                          // No interrupts
    case 2 : break;                          // RTCRDYIFG
    case 4 : break;                          // RTCEVIFG  // event happened.
    case 6 :
    		/* Alarm at sunday happened */
			 ucRTCAlarmSetFlag = TRUE;

			 /* Stop the Clock */
			 TS_TimerPwm_Stop();
			 __bic_SR_register_on_exit(LPM3_bits);
			 __no_operation();
    		 break;                          // RTCAIFG

    case 8 : break;                          // RT0PSIFG
    case 10: break;                          // RT1PSIFG
    case 12: break;                          // Reserved
    case 14: break;                          // Reserved
    case 16: break;                          // Reserved
    default: break;
  }
}
