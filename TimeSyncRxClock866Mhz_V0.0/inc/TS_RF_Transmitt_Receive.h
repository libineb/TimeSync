/*
 * TS_RF_Transmitt_Receive.h
 *
 *  Created on: Mar 19, 2013
 *      Author: libin.eb
 */
#ifndef TS_RF_TRANSMITT_RECEIVE_H_
#define TS_RF_TRANSMITT_RECEIVE_H_

/* Header files */
#include <stdlib.h>     				 /* atoi functions */
#include <string.h>						 /* For string functions*/
#include "ctype.h"
#include "cc430x513x.h"

#include "RF1A.h"
#include "hal_PMM.h"
#include "hal_UCS.h"
/*User defined header files*/
#include "TS_Timer_Pwms.h"
#include "common.h"

/* Macros */
#define PACKET_LEN_MAX         				(64)			 // FIFO size
#define PACKET_LEN         			 		(63)	    	 // PACKET_LEN > FIFO Size (64) (62)
/* status bytes */
#define RSSI_IDX           			 		(PACKET_LEN + 1)  // Index of appended RSSI
#define CRC_LQI_IDX        			 		(PACKET_LEN + 2)  // Index of appended LQI, checksum
#define CRC_OK             			 		(BIT7)            // CRC_OK bit
#define PATABLE_VAL        			 		(0x51)            // 0 dBm output
#define RX_BYTES_MASK						0x7F

#define GPS_LINK_LOSS_CHECK_MINUTE		 	5				// Total minutes that the radio will be in RX mode
#define NUMBER_OF_LINKLOSS				 	2				// times the number of link loss will occur
/*Macros for the rx delay timer*/
#define RX_ON_MODE						 	1
#define RX_OFF_MODE						 	2
#define RXTIMER_COUNT					 	(4096)			// counter value for 1 sec
#define RX_ON_SECONDS					 	20				// for check changed 5
#define RX_OFF_SECONDS					 	(SECOND_MAX - RX_ON_SECONDS) // total period is 1 minute(RX On and RX OFF)

/*Macros for GPS time calculation*/
#define GMT_HOUR  	            		 	5
#define GMT_MINUTE  	        		 	30
#define HOUR_MAX  						 	12
#define HOUR_DAY_MAX  		   			 	24
#define MINUTE_MAX  	    			 	60
#define SECOND_MAX						 	60
#define HOUR_TOTAL_SECONDS  			 	3600
#define MINUTE_TOTAL_SECONDS  			 	60
/*Macros for NMEA message format process*/
#define DOLLAR_CHARACTER				 	'$'
#define GPRMC_MESSAGE   				 	"$GPRMC,"
#define GPGGA_MESSAGE 				     	"$GPGGA,"
#define VALID_CHARACTER			 	     	'A'
#define INVALID_CHARACTER			 	 	'V'
#define NO_BYTES_IN_MESSAGE				  	7
#define COMMA_CHARACTER					 	','
#define DATA_VALID_CHARACTER			 	0x11
#define HOUR_FIRST_BYTE   				 	0x07 			/*for detecting the hour data*/
#define HOUR_SECOND_BYTE  				 	0x08
#define MINUTE_FIRST_BYTE  				 	0x09 			/*for detecting the minute data*/
#define MINUTE_SECOND_BYTE  			 	0x0A
#define SECOND_FIRST_BYTE  				 	0x0B 			/*for detecting the second data*/
#define SECOND_SECOND_BYTE  			 	0x0C
/* Macros for the day of the week calculation */
#define DAY_FIRST_BYTE_A			     	0x35			// 53
#define DAY_SECOND_BYTE_A				 	0x36
#define MONTH_FIRST_BYTE_A				 	0x37
#define MONTH_SECOND_BYTE_A				 	0x38
#define YEAR_THIRD_BYTE_A				 	0x39
#define YEAR_FOURTH_BYTE_A				 	0x3A			// 58
#define DAY_FIRST_BYTE_V				 	0x19			// 25
#define DAY_SECOND_BYTE_V				 	0x1A
#define MONTH_FIRST_BYTE_V				 	0x1B
#define MONTH_SECOND_BYTE_V				 	0x1C
#define YEAR_THIRD_BYTE_V				 	0x1D
#define YEAR_FOURTH_BYTE_V				 	0x1E			// 30

#define GPS_VALID						 	1
#define GPS_INVALID						 	2
/*Macros for Initial Time setting,*/
#define INITIAL_HOUR			  		 	0				// 0 for 12 '0' clock
#define INITIAL_MINUTE			   		 	0
#define INITIAL_SECONDS			  		 	0
//#define INITIAL_TIME_SECONDS  	  	 	((INITIAL_HOUR * 3600) + (INITIAL_MINUTE * 60) + INITIAL_SECONDS)

#define RUNNING_DELAY_PARAMETER     	 	(0.001 * FAST_MODE_TIME) 	// For One tick it will take 50msec

#define GPS_DATA_ERROR            		 	(43200)
#define TOTAL_SECONDS_12HOUR			 	(43200)
#define MAX_SECONDS_OF_GPS				 	(43199)
#define TIME_RANGE							(41400)     /*Means total seconds for 11.45(42300), time before the 12 o clock*/

/* Macro for checking  weather the year is leap year or not */
#define IS_LEAP_YEAR(x)          ((((x) % 400 == 0) || (((x) % 4 == 0) && ((x) % 100 != 0))) ? TRUE : FALSE)
#define MAX_WEEK_DAY					 	7
#define ZERO_ODD_DAYS					 	0
#define ONE_ODD_DAYS					 	1
#define THREE_ODD_DAYS					 	3
#define FIVE_ODD_DAYS					 	5
#define LEAP_YEAR_DURATION				 	4
#define MAXMONTH						 	12
#define ZERO_ODD_DAYS_REF_YEAR_2012			2012
#define ZERO_ODD_DAYS_REF_YEAR_2000			2000
#define ZERO_ODD_DAYS_DURATION_YEARS_400	400
#define ONE_ODD_DAYS_DURATION_YEARS_300		300
#define THREE_ODD_DAYS_DURATION_YEARS_200	200
#define FIVE_ODD_DAYS_DURATION_YEARS_100	100
#define CURRENT_CENTURY						2000

#define FEBRUARY_LEAP_DAYS					29
#define FEBRUARY_NORMAL_DAYS				28

#define TX_IN_RANGE_MASK					0x0F
#define TX_IN_RANGE_BYTE					0xF0

#define RAD_RECIEVED_MASK					0xF0
#define RAD_RECIEVED_BYTE					0x0F

#define RADIO_STATE_MASK					0x70
#define RADIO_RX_STATE						0x10
#define RADIO_IDLE_STATE					0x00

/* Extern variable */
extern unsigned char ucRESETInterrupt;

/*******************
 * Function Definition
 */
extern void ReceiveOff(void);
extern void TS_GPSDataHandler(void);
extern uint16 TS_ReturnActualSecondsToRotate(void);

uint16 TS_RadioReception(void);				// for checking purpose made extern

#endif /* TS_RF_TRANSMITT_RECEIVE_H_ */
