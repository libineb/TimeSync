/*
 * TS_RTC.h
 *
 *  Created on: May 9, 2013
 *      Author: libin.eb
 */

#ifndef TS_RTC_H_
#define TS_RTC_H_

/* Device specific header file */
#include "cc430f5137.h"
/* User includes */
#include "common.h"
#include "TS_Timer_Pwms.h"

/* RTC alarm set mask */
#define ALARM_SET_MASK				0x80
/* Masking to avoid invalid alarm setting */
#define ALARM_MIN_MASK				0x3F
#define ALARM_HOUR_MASK				0x1F
#define ALARM_DOW_MASK				0x07
#define ALARM_DAY_MASK				0x1F

#define RTC_HOUR_MAX				24
#define RTC_MINUTE_MAX				60
#define RTC_SECONDS_MAX				60
#define RTC_DOW_MAX					6
#define RTC_SET_COUNTER				7	// Counter for the RTC that will set the GPS sync
										// Sync is set after 7 days.

/* Use defined functions */
extern uint8 TS_RTC_RegisterSetStart(uint8 ucHour,uint8 ucMinute,uint8 ucSeconds,uint8 ucDOW );
extern void TS_RTC_ReadStop(void);
extern void TS_RTC_SetAlarm(void);
extern uint8 TS_RTCReturnIntFlag(void);
extern void TS_RTCClearIntFlag(void);

#endif /* TS_RTC_H_ */
