/*
 * TS_PinConfiguration.h
 *
 *  Created on: May 9, 2013
 *      Author: libin.eb
 */

#ifndef TS_PINCONFIGURATION_H_
#define TS_PINCONFIGURATION_H_

/* Device specific header file */
#include "cc430f5137.h"

/* User defined header file */
#include "TS_Proximity_Sensor.h"
#include "TS_Timer_Pwms.h"
#include "TS_RF_Transmitt_Receive.h"

/*Extern variables declarations*/
extern volatile unsigned char ucFirstInterruptPin;
extern volatile unsigned char ucSecondInterruptPin;

/* Macros */
#define GREEN_LED_PIN							BIT0		// for checking purpose
#define RED_LED_PIN								BIT6		// For low power indication
#define AMBER_LED_PIN							BIT6		// For link loss indication

#define GREEN_LED_ON							(P1OUT |= GREEN_LED_PIN)
#define GREEN_LED_OFF							(P1OUT &= ~GREEN_LED_PIN)
#define RED_LED_ON								(P3OUT |= RED_LED_PIN)
#define RED_LED_OFF								(P3OUT &= ~RED_LED_PIN)

/* User defined functions */
extern void TS_LEDInit(void);
extern void TS_RESETSwitchInit(void);

#endif /* TS_PINCONFIGURATION_H_ */
