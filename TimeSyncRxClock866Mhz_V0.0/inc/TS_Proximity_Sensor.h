/*
 * TS_Proximity_Sensor.h
 *
 *  Created on: Apr 23, 2013
 *      Author: libin.eb
 */

#ifndef TS_PROXIMITY_SENSOR_H_
#define TS_PROXIMITY_SENSOR_H_

/* Header files */
#include "string.h"
#include "common.h"
#include "cc430f5137.h"

/* macros for the hardware pins using for IIC communication */
#define MINUTE_INT					BIT4
#define MINUTE_INT_DIR			    P1DIR
#define MINUTE_INT_REN			    P1REN
#define MINUTE_INT_IES			    P1IES
#define MINUTE_INT_OUT			    P1OUT
#define MINUTE_INT_IE			    P1IE
#define MINUTE_INT_IFG			    P1IFG

#define MINUTE_SCL					BIT2
#define MINUTE_SDA					BIT3
#define MINUTE_IIC_SEL			    P1SEL

#define HOUR_INT					BIT4
#define HOUR_INT_DIR			    P2DIR
#define HOUR_INT_REN			    P2REN
#define HOUR_INT_IES			    P2IES
#define HOUR_INT_OUT			    P2OUT
#define HOUR_INT_IE			        P2IE
#define HOUR_INT_IFG			    P2IFG

#define HOUR_SCL					BIT6
#define HOUR_SDA					BIT3
#define HOUR_IIC_SEL				P2SEL

/*Wait time calculation for minute hand*/
#define FAST_CLOCK					0.05
#define MINUTE_HAND_LENGTH			105
#define SENSOR_WIDTH				2.73
#define TIME_TO_COVER				(((3600 * FAST_CLOCK)/(2 * 3.14 * MINUTE_HAND_LENGTH)) * (SENSOR_WIDTH * 0.75))
#define WAIT_REG_VALUE				(TIME_TO_COVER / 0.032)

#define SLAVE_ADDRESS      			0x39		// slave address of the sensor
/*command for registers*/
#define COMMAND_ENABLE_REG		 	0x80		// enable register
#define COMMAND_ID_REG				0x92		// device Id register ID is 0x39
#define COMMAND_CONTROL_REG 		0x8F		// Control register
#define COMMAND_PULSE_COUNT_REG     0x8E		// Number of pulses register
#define COMMAND_PERSISTENCE_REG     0x8C		// Number of proximity readings for the interrupt generation
#define COMMAND_STATUS_REG			0x93		// Sensor status register
#define COMMAND_PROX_READ_REG      	0xB8		// auto increment selected
#define COMMAND_WAIT_TIME_REG		0x83		// Wait time after proximity detection
#define COMMAND_OFFSET_REG			0x9E		// Offset value set register
#define COMMAND_INT_CLEAR_SPECIAL	0xE5		// special command for disabling all the interrupts
#define COMMAND_CONFIG_REG			0x8D		// Configuration register for wait time

#define COMMAND_LOW_HIGH_BYTE_REG 	0x89		// Low byte threshold register
#define COMMAND_LOW_LOW_BYTE_REG 	0x88
#define COMMAND_HIGH_HIGH_BYTE_REG 	0x8B		// High byte threshold register
#define COMMAND_HIGH_LOW_BYTE_REG 	0x8A

/*Data to be written to Sensor registers*/
/*configuration for the second hand sensor*/
#define TOTAL_COMMANDS_SECOND		6
#define PULSE_COUNT_SECOND			0x01 		// for 1 pulses
#define DIODE_GAIN_SECOND 			0xE0		// selecting the CH0 diode and gain * 1

#define INT_HIGH_HIGH_SECOND		0x00		// High threshold value for Second sensor
#define INT_HIGH_LOW_SECOND			0x38		// 0x88  0xF6 0x20
#define INT_LOW_HIGH_SECOND			0x00
#define INT_LOW_LOW_SECOND			0x99

/*configuration for the Hour hand sensor*/
#define TOTAL_COMMANDS_HOUR			5
#define PULSE_COUNT_HOUR			0x01 		// for 1 pulses
#define DIODE_GAIN_HOUR 			0xD0		// selecting the CH0 diode and gain * 1

#define INT_HIGH_HIGH_HOUR			0x03		// High threshold value for Second sensor 0x02
#define INT_HIGH_LOW_HOUR			0x24		// 0x88 0x880x24
#define INT_LOW_HIGH_HOUR			0x00
#define INT_LOW_LOW_HOUR			0x99

#define HOUR_THRESHOLD_VALUE		0x0300		// 0x0324

/*configuration for the Minute hand sensor*/
#define TOTAL_COMMANDS_MINUTE		5
#define PULSE_COUNT_MINUTE			0x01 		// for 1 pulses
#define DIODE_GAIN_MINUTE 			0xD0		// selecting the CH0 diode and gain * 1
#define INT_HIGH_HIGH_MINUTE		0x01		// High threshold value for First sensor 0x01

#define INT_HIGH_LOW_MINUTE			0xA9        // 0x00
#define INT_LOW_HIGH_MINUTE			0x00
#define INT_LOW_LOW_MINUTE			0x00

#define MINUTE_THRESHOLD_VALUE		0x020E

#define PERSISTANCE_VALUE 			0x10        // checking 1 consecutive values
#define OFFSET_VALUE				0x00		// subtract this value 0xD0
#define CONFIG_VALUE				0x02		// WLONG enabled
#define WAIT_TIME_SECOND			0xFF		// For 1 second delay  0xE1

#define COMMAND_STATUS_INT_CLEAR	0x00		// Special command for clearing all interrupts

#define PON_PROX_ENABLE     		0x05       	// Power ON, proximity.
#define PON_INT_PROX_ENABLE			0x25		// Power On, Proximity and Interrupt enabled
#define PON_INT_PROX_SLEEP_ENABLE	0x65		// Power On, Proximity, Interrupt and sleep
#define PON_INT_PROX_WAIT_ENABLE	0x2D		// Power On, Proximity, Interrupt and wait

#define TOTAL_BYTES_SEND			2
#define TOTAL_NO_READINGS			5

#define HOUR_HAND					0
#define SECOND_HAND					1

#define SHIFT						8

#define INTERRUPT_ENABLE			0
#define INTERRUPT_DISABLE			1

#define CLEAR_COMMAND_BYTE			1

/*User defined functions*/
extern void TS_MinuteSensorStart(unsigned char ucInterrupt);
extern void TS_HourSecondSensor_Start(unsigned char ucSelectHand,unsigned char ucInterrupt);
extern void TS_Sensor_Stop(void);
extern void TS_SensorsInterrupt_Clear(void);
extern unsigned short TS_ReturnProximityReadData(void);

extern unsigned char TS_Sensor_IIC_Read (const unsigned char *ucWriteBuffer, unsigned char *ucReadBuffer);
extern unsigned char TS_Sensor_IIC_Write (const unsigned char *ucWriteBuffer, unsigned char ucTotalBytes);

#endif /* TS_PROXIMITY_SENSOR_H_ */
