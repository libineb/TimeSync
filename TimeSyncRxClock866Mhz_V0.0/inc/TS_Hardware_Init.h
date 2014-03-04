/*
 * TS_Hardware_Init.h
 *
 *  Created on: Apr 18, 2013
 *      Author: libin.eb
 */
#ifndef TS_HARDWARE_INIT_H_
#define TS_HARDWARE_INIT_H_

/* Device specific header file */
#include "cc430f5137.h"
/* Header Files */
#include "hal_UCS.h"
#include "TS_Proximity_Sensor.h"
#include "TS_Timer_Pwms.h"
#include "TS_PinConfiguration.h"
#include "TS_RTC.h"
#include "common.h"

/* Macros */
#define LOW_POWER_LED				BIT5

/* extern functions */
extern void TS_HardwareInit(void);

#endif /* TS_HARDWARE_INIT_H_ */
