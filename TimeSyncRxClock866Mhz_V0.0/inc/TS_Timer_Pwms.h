/*
 * TS_Timer_Pwms.h
 *
 *  Created on: Mar 22, 2013
 *      Author: libin.eb
 */
#ifndef TS_TIMER_PWMS_H_
#define TS_TIMER_PWMS_H_

/* Device header file */
#include "cc430f5137.h"
/* User header files */
#include "hal_UCS.h"
#include "TS_PinConfiguration.h"
#include "TS_RTC.h"
#include "TS_Proximity_Sensor.h"
#include "common.h"

/* Macros for the functions */
#define PWM_COIL1									BIT0
#define PWM_COIL2									BIT2

#define FRQ_MULTIPLIER								32.768	// using 32.768Khz clock
#define NORMAL_MODE_TIME							1000	// in mesec(1sec)
#define FAST_MODE_TIME								50		// in 50msec
#define SECOND_SENSE_TIME							60		// 60msec

#define ON 						  					1
#define OFF						  					0

#define SECONDS_OF_FIRSTSENSOR_START				3300	// 55 Minutes
//#define SECOND_SENSOR_PROXI_VALUE					0x0200

/* Macros for PWM generation */
#define TRANSISTOR_PULL								1		// With transistor pull

/*Macros for the Timer operation*/
#define CCR0_ONE_SEC_PERIOD     					((NORMAL_MODE_TIME * FRQ_MULTIPLIER) - 1)  	// for 1sec
#define ONE_SEC_ON									(25 * FRQ_MULTIPLIER)

#define CCR0_FAST_SEC_PERIOD    					((FAST_MODE_TIME * FRQ_MULTIPLIER) - 1) 	// for 50msec 1638.4 (1638-1)
#define FAST_SEC_ON									(25 * FRQ_MULTIPLIER)

#define CCR0_SECOND_FAST_PERIOD    					((SECOND_SENSE_TIME * FRQ_MULTIPLIER) - 1)  // for 50msec 1638.4 (1638-1)
#define SECOND_FAST_SEC_ON							(25 * FRQ_MULTIPLIER)

#ifdef TRANSISTOR_PULL
	#define CCR1_FIRST_DUTYCYCLE      			    (CCR0_ONE_SEC_PERIOD - ONE_SEC_ON)
	#define CCR2_FIRST_DUTYCYCLE       			    (CCR0_ONE_SEC_PERIOD - ONE_SEC_ON)          // for 25msec 819.2 for 2nd PWM
#else
	#define CCR1_FIRST_DUTYCYCLE      			    ONE_SEC_ON        	// for 25msec, transistor drive
	#define CCR2_FIRST_DUTYCYCLE       			    ONE_SEC_ON       	// for 25msec,
#endif

#define CCR1_FIRST_DUTYCYCLE_FAST      				(CCR0_FAST_SEC_PERIOD - FAST_SEC_ON)
#define CCR2_FIRST_DUTYCYCLE_FAST       			(CCR0_FAST_SEC_PERIOD - FAST_SEC_ON)

//#define CCR1_SECOND_DUTYCYCLE      				0
//#define CCR2_SECOND_DUTYCYCLE      				0

#define CCR1_SECOND_OFF								(CCR0_ONE_SEC_PERIOD + 100)   	// any value greater than the frequency
#define CCR2_SECOND_OFF								(CCR0_ONE_SEC_PERIOD + 100)

#define CCR1_FAST_OFF								(CCR0_FAST_SEC_PERIOD + 100)   // any value greater than the fast frequency
#define CCR2_FAST_OFF								(CCR0_FAST_SEC_PERIOD + 100)

typedef enum
	{
		TS_OFFMode=0,
		TS_NormalMode,
		TS_FastMode,
		TS_InitialSetMode,
		TS_AheadMode
	}TS_PWMmodes;
extern TS_PWMmodes ucPwmMode;

/*User defined functions*/
extern void TS_TimerPwm_Init(void);
extern void TS_TimerPwm_Mode(TS_PWMmodes ucModeSelection);
extern void TS_TimerPwm_Stop(void);

#endif /* TS_TIMER_PWMS_H_ */
