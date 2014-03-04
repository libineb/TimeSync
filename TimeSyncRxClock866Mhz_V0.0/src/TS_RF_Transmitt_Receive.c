/****************************************************************************
 * TS_RF_Transmitt_Receive.c
 *
 *  Created on: Nov 19, 2014
 *      Author: libin.eb
 *
******************************************************************************/
/* Header Files */
#include "TS_RF_Transmitt_Receive.h"

/* extern structure for setting the radio registers */
extern RF_SETTINGS rfSettings;

/* variables for GPS data process */
/* structure for storing the GPS time data */
typedef struct GPS_Data
	{
		uint8 ucGPSDataValid;
		uint16 usTotalGPSSeconds;
	}st_GPS_Data;
/* Structure for storing date,month and year */
typedef struct DateTime
	{
		char ucDay; 	// day of month
		char ucMon; 	// month
		short usYear; 	// year
	}DateTime_st;
/* +2 for status bytes first byte RSSI value and 2nd byte CRC and LQI*/
volatile uint8 receiving = 0;
/* maximum packet length will be 63 */
static int8 ucGPSDataReceived[PACKET_LEN] = {0};
/* variable for exact number of seconds needed */
static uint16 usActualSecondsNeeded = 0;

static uint16 usInitialTotalSeconds = 0;	// check

/* For Hour,Minute,Second,Day, month and year value from the GPS, should be global*/
static uint16 usHour_data = 0;						// make as local
static uint16 usMinute_data = 0;
static uint16 usSecond_data = 0;
static uint16 usDate_data = 0;
static uint16 usMonth_data = 0;
static uint16 usYear_data = 0;

static uint16 usDelaysecCount = 0;
static uint16 usDelaysecs = 0;
static uint8 ucRXDelayModeSelect = 0;
static uint8 ucGPSLinkLossCheck = 0;

/* User defined functions */
void ReceiveOn(void);
void InitRadio(void);
static void TS_RFReceptionRadioInit (void);
static st_GPS_Data TS_ReceiveBufferProcess (void);
static st_GPS_Data TS_DataProcessRMC_GGA (uint8 ucGPSValid,uint8 ucFirstNumber);
static uint16 TS_TotalSecondsCalc (uint16 usGpsHour,uint16 usGpsMinute,\
										uint16 usGpsSecond);
static uint8 TS_GetOddDays(DateTime_st  stDateMonYear);
static uint16 TS_RunningDelayCalc (uint16 usTotalSeconds);
static uint8 TS_RXDelayWaitStart (uint16 usDelaysec, uint8 ucRXDelayMode);
static void TS_RXDelayWaitInit (void);
static void TS_RXDelayWaitStop (void);

/**************************************************************************//**
* TS_RadioReception
* @brief         - Program for receiving the RF signals at 33.622K baud rate.
* @param         -
* @return        -
******************************************************************************/
uint16 TS_RadioReception(void)
{
  uint8 ucDataReceiveStart = TRUE;
  ucGPSLinkLossCheck = 0;
  st_GPS_Data GPSProcessStruct;
  volatile uint8 ucRadioStatusCheck = 0;

  receiving = 0;

  /* Initialization radio registers */
  TS_RFReceptionRadioInit();
  TS_RXDelayWaitInit();

  while (ucDataReceiveStart)
  {
	 /* Starting the reception */
	 ReceiveOn();
	 while(!(((ucRadioStatusCheck = Strobe(RF_SNOP)) & RADIO_STATE_MASK) == RADIO_RX_STATE));

	 /* Starting the delay timer */
	 TS_RXDelayWaitStart(RX_ON_SECONDS,RX_ON_MODE);

	__bis_SR_register(LPM3_bits + GIE);
	__no_operation();

	 /* Variable set in the ISR of radio */
	 if((receiving & RAD_RECIEVED_BYTE))
	 {
		receiving &= RAD_RECIEVED_MASK;
		/* make LEDs OFF if any one is ON */
		RED_LED_OFF;											// for check
		GREEN_LED_OFF;

		/* GPS buffer is processed and assign the total GPs seconds and valid data byte to a structure */
		GPSProcessStruct = TS_ReceiveBufferProcess();
		/* Checking the validity of the Received GPS data */
		if(GPSProcessStruct.ucGPSDataValid == TRUE)
		{
			ucDataReceiveStart = OFF;	// exiting the while loop.
			ucGPSLinkLossCheck = 0;
//			GREEN_LED_ON; 				// for check, valid data received indication.
//			TS_RXDelayWaitStop();		// for check no need

			ReceiveOff();
			ResetRadioCore();			// SRES command put the radio in sleep state.

//			while(1);					// for check
		}
		else
		{
			ucDataReceiveStart = ON;
			GPSProcessStruct.usTotalGPSSeconds = FALSE;
		}
	}
    /* Link loss checking,*/
	/* Radio will check every 2 second and after 10 second(Duty Cycle) it will be off for 50 second(total duration 1 Minute)*/
	/* the Reception will be OFF and exit after 5 minutes */
	else
	  {
		  /* reception OFF for 50sec */
		  ReceiveOff();
		  while(!(((ucRadioStatusCheck = Strobe(RF_SNOP)) & RADIO_STATE_MASK) == RADIO_IDLE_STATE));

		  TS_RXDelayWaitStart(RX_OFF_SECONDS,RX_OFF_MODE);

		 __bis_SR_register(LPM3_bits + GIE);
		 __no_operation();

		  if(ucGPSLinkLossCheck == (uint8)(GPS_LINK_LOSS_CHECK_MINUTE))
		  {
			  ucGPSLinkLossCheck = 0;
			  /* stopping the reception after 5 minutes.*/
			  ucDataReceiveStart = FALSE;
			  GPSProcessStruct.usTotalGPSSeconds = GPS_DATA_ERROR;

			  ReceiveOff();						  			// check whether needed or not  already in interval check
			  ResetRadioCore();

			  if(!(receiving & TX_IN_RANGE_BYTE))
			  {
				 while(1)
				 {
					RED_LED_OFF;
					__delay_cycles(100000);
					RED_LED_ON;
					__delay_cycles(100000);
				 }
			  }
		  }
	  }
   }

  return GPSProcessStruct.usTotalGPSSeconds;
}
/**************************************************************************//**
* TS_GPSDataHandler
* @brief         - Program for handling the GPS data and run the clock.
* 				   All the processes are doing here.
* @param         -
* @return        -
******************************************************************************/
void TS_GPSDataHandler(void)
{
	uint16 usGPS_TotalSeconds = 0;
	uint16 usRunningDelayCount = 0;
	uint8 ucTotal_Linkloss_Count = 0;
	uint8 ucClockAhead = 0;

	/* Infinite loop that will do time syncing and clock setting */
	while(1)
	{
		usGPS_TotalSeconds = 0;
		usRunningDelayCount = 0;
		ucClockAhead = 0;
		usActualSecondsNeeded = 0;

		/* Every time when clock is out from normal mode initial set is needed */
		TS_TimerPwm_Mode(TS_InitialSetMode);

		/* taking the total GPS data seconds */
		/* This will configure the radio registers and enable RX for receiving the GPS data.
		 * The processed GPS data will convert in to total number of seconds and will return.
		 */
		usGPS_TotalSeconds = TS_RadioReception();

		/* checking the valid data, error value is "GPS_DATA_ERROR" */
		if(usGPS_TotalSeconds < GPS_DATA_ERROR)
		{
			/* Normal operation will happen if the time is up to 11.45(TIME_RANGE) */
			/* It is normal condition */
			if((usGPS_TotalSeconds <= TIME_RANGE) && (usGPS_TotalSeconds != 0))
			{
				usRunningDelayCount = TS_RunningDelayCalc(usGPS_TotalSeconds);		      // calculating running delay
				usActualSecondsNeeded = (usGPS_TotalSeconds + usRunningDelayCount);    // Actual seconds to rotate
			}
			/* when clock is ahead */
			else if((TIME_RANGE < usGPS_TotalSeconds) &&  (usGPS_TotalSeconds <= MAX_SECONDS_OF_GPS))
			{
				/* Total seconds of ahead */
				usInitialTotalSeconds = (TOTAL_SECONDS_12HOUR - usGPS_TotalSeconds);

				/* clock will stop up to this time */
				usActualSecondsNeeded = (usInitialTotalSeconds);

				ucClockAhead = TRUE;
			}
			/* At 12'O' clock or clock is in sync */
			else if(usGPS_TotalSeconds == 0)
			{
				usActualSecondsNeeded = 0;
			}
			/* any error condition will continue the normal operation */
			else
			{
				usActualSecondsNeeded = 0;
			}
			/*case will happen after including the delay value, it may more than the 12 clock value seconds to rotate*/
			if(usActualSecondsNeeded >= TOTAL_SECONDS_12HOUR)
			{
				// usActualSecondsNeeded = (usActualSecondsNeeded - TOTAL_SECONDS_12HOUR);
				// yet to include
				__no_operation();
			}
			/* Will select the clock operation according total number of clock seconds needed and start RTC alarm */
			if(usActualSecondsNeeded)
			{
				if(ucClockAhead)
				{
					GREEN_LED_ON;							// time is syncing indication
					TS_TimerPwm_Mode(TS_AheadMode);
					ucClockAhead=0;
					GREEN_LED_OFF;							// syncing happened

					TS_RTC_SetAlarm();						// start the RTC alarm here.
					TS_TimerPwm_Mode(TS_NormalMode); 		// added
				}
				else
				{
					GREEN_LED_ON;							// time is syncing indication
					TS_TimerPwm_Mode(TS_FastMode);
					GREEN_LED_OFF;							// syncing happened
					__no_operation();

					TS_RTC_SetAlarm();						// start the RTC alarm here.
					TS_TimerPwm_Mode(TS_NormalMode);
				}
			}
			else											// time is 12 'o'clock or clock is in sync
			{
				TS_RTC_SetAlarm();						    // start the RTC alarm here.
				TS_TimerPwm_Mode(TS_NormalMode);
			}
		}
		else
		{
		  ucTotal_Linkloss_Count++;
		  if(ucTotal_Linkloss_Count == NUMBER_OF_LINKLOSS)  // total number of link losses is 2.
		  {
			  ucTotal_Linkloss_Count = 0;

			  RED_LED_ON;  									// Link loss indicate by glowing the RED LED.

			  while(1);					    				// Trap here when link loss occurs
		  }
		  else
		  {
			  /* Since 5 minutes is waited clock should run to set the 5 minutes wait time */
			  /* Radio will be in RX this much time. If the data is not getting then the clock need to be updated */
			  usActualSecondsNeeded = (uint16)(GPS_LINK_LOSS_CHECK_MINUTE * MINUTE_TOTAL_SECONDS);
			  TS_TimerPwm_Mode(TS_FastMode);

			  /*This condition means no valid GPS data got. So already the clock is in the 24 'o' clock condition
			   * so update the RTC with that value.
			   */
			  TS_RTC_RegisterSetStart(0,(uint8)GPS_LINK_LOSS_CHECK_MINUTE,0,0);
			  /*start the alarm by setting the RTC initial time as 24 'o' clock*/
			  TS_RTC_SetAlarm();

			  /*Continue the normal mode of operation*/
			  TS_TimerPwm_Mode(TS_NormalMode);
		  }
		}
	}
}
/**************************************************************************//**
* TS_ReturnActualSecondsToRotate
* @brief         - Program for returning Actual seconds for the clock
* 				   should rotate.
* @param         -
* @return        -
******************************************************************************/
uint16 TS_ReturnActualSecondsToRotate (void)
{
  return usActualSecondsNeeded;
}
/**************************************************************************//**
* TS_RFReceptionRadioInit
* @brief         - Program for initialization radio reception.
* @param         -
* @return        -
******************************************************************************/
static void TS_RFReceptionRadioInit (void)
{
  /* Increase PMMCOREV level to 2(core voltage to 1.8V).*/
  SetVCore(2);
  /*Reset all the radio registers*/
  ResetRadioCore();
  /*Initialization of the Radio module, configuring the registers*/
  InitRadio();
}
/**************************************************************************//**
* ReceiveOn
* @brief         - Program for starting the reception.
* @param         -
* @return        -
******************************************************************************/
void ReceiveOn(void)
{
  /* Sync word sent or received */
  RF1AIES |= BIT9;						// RF1AIES &= ~BIT9(Sync word sent or received);
  	  	  	  	  	  	  	  	  	  	// RF1AIES |= BIT9(interrupt for receiving the packet);
  RF1AIFG  = 0;                         // Clear pending RFIFG interrupts
  RF1AIE  |= BIT9;                      // Enable the sync word received interrupt

  // Radio is in IDLE following a TX, so strobe SRX to enter Receive Mode
  Strobe( RF_SRX );
  __no_operation();
}
/**************************************************************************//**
* ReceiveOff
* @brief         - Program for stopping the reception.
* @param         -
* @return        -
******************************************************************************/
void ReceiveOff(void)
{
  RF1AIE = 0;
  RF1AIFG = 0;
  RF1AIES &= ~BIT9;                         // Switch back to to sync word

  // It is possible that ReceiveOff is called while radio is receiving a packet.
  // Therefore, it is necessary to flush the RX FIFO after issuing IDLE strobe
  // such that the RXFIFO is empty prior to receiving a packet.
  Strobe(RF_SIDLE);
  Strobe(RF_SFRX);
}
/**************************************************************************//**
* InitRadio
* @brief         - Program for initialization of Radio.
* @param         -
* @return        -
******************************************************************************/
void InitRadio(void)
{
  // Set the High-Power Mode Request Enable bit so LPM3 can be entered
  // with active radio enabled
  /* PMM password. Always read as 096h. When using word operations, must be
	written with 0A5h or a PUC is generated. When using byte operation, writing
	0A5h unlocks all PMM registers. When using byte operation, writing anything
	different than 0A5h locks all PMM registers */
  PMMCTL0_H = 0xA5;
  PMMCTL0_L |= PMMHPMRE_L;
  PMMCTL0_H = 0x00;

  /* all the radio core configuration register settings are done here*/
  WriteRfSettings(&rfSettings);
  /* Out put power setting */
  WriteSinglePATable(PATABLE_VAL);
}
/******************************************************************************
* Outline 		: TS_ReceiveBufferProcess
* Description 	: Program for processing the GPS data received.
* Argument  	: None
* Return value  : Total seconds corresponds to UTC time.
******************************************************************************/
static st_GPS_Data TS_ReceiveBufferProcess(void)
{
	uint8 ucDataCheck = 0;
	uint8 ucDollar_Detect = 0;
	int8 *Message_String = NULL;
	int8 CProcessBuffer[PACKET_LEN] = {0};
	uint8 ucBufferCheck = 0;
	int ucDigitCheckTime = 0;
	int ucDigitCheckDate = 0;
	st_GPS_Data GPSProcessDataStruct;
	uint8 ucGPSValid = 0;

	memcpy(CProcessBuffer,ucGPSDataReceived,PACKET_LEN);

   	for(ucDataCheck=0; ucDataCheck < PACKET_LEN; ucDataCheck++)
   	{
		/* checking for the '$' int8acter detect */
	  	if(CProcessBuffer[ucDataCheck] == DOLLAR_CHARACTER)
	  	{
			ucDollar_Detect = ucDataCheck;

			Message_String = &CProcessBuffer[ucDollar_Detect];

			 /* if Return value if = 0 then it indicates string1 is equal to string2 */
			if((!(strncmp(Message_String,GPRMC_MESSAGE,NO_BYTES_IN_MESSAGE))) && ((ucDollar_Detect+YEAR_FOURTH_BYTE_A) < PACKET_LEN))
			{
				for(ucBufferCheck = (ucDollar_Detect+HOUR_FIRST_BYTE);ucBufferCheck <=(ucDollar_Detect+SECOND_SECOND_BYTE);ucBufferCheck++)
				{
					ucDigitCheckTime = isdigit(CProcessBuffer[ucBufferCheck]);

					/*If any of the scanned byte is non digit*/
					if(ucDigitCheckTime == 0)
					{
						break;
					}
					/* all are digit values */
					else
					{
						Message_String = NULL;
					}
				}
				if(CProcessBuffer[ucDollar_Detect + DATA_VALID_CHARACTER] == VALID_CHARACTER)
				{
					for(ucBufferCheck =(ucDollar_Detect+DAY_FIRST_BYTE_A);ucBufferCheck <=(ucDollar_Detect+YEAR_FOURTH_BYTE_A);ucBufferCheck++)
					{
						ucDigitCheckDate = isdigit(CProcessBuffer[ucBufferCheck]);

						if(ucDigitCheckDate == 0)
						{
							break;
						}
						else
						{
							ucGPSValid = GPS_VALID;
							Message_String = NULL;
						}
					}
				}
				if(CProcessBuffer[ucDollar_Detect + DATA_VALID_CHARACTER] == INVALID_CHARACTER)
				{
					for(ucBufferCheck =(ucDollar_Detect+DAY_FIRST_BYTE_V);ucBufferCheck <=(ucDollar_Detect+YEAR_FOURTH_BYTE_V);ucBufferCheck++)
					{
						ucDigitCheckDate = isdigit(CProcessBuffer[ucBufferCheck]);

						if(ucDigitCheckDate == 0)
						{
							break;
						}
						else
						{
							ucGPSValid = GPS_INVALID;
							Message_String=NULL;
						}
					}
				}
				if((ucDigitCheckTime) && (ucDigitCheckDate))
				{
					GPSProcessDataStruct = TS_DataProcessRMC_GGA(ucGPSValid,ucDollar_Detect);
					if(GPSProcessDataStruct.ucGPSDataValid == TRUE)
					{
						break;
					}
					else
					{
						ucGPSValid = 0;
						Message_String = NULL;
						continue;
					}
				}
				else
				{
//					ucDollar_Detect=FALSE;
					Message_String = NULL;
					continue;
				}
			}
			else
			{
				GPSProcessDataStruct.ucGPSDataValid = FALSE;
				GPSProcessDataStruct.usTotalGPSSeconds = FALSE;
				Message_String = NULL;
				continue;
			}
	  	}
    }
	memset(ucGPSDataReceived,0,PACKET_LEN);

	return GPSProcessDataStruct;
}

/******************************************************************************
Name       : TS_TotalSecondsCalc
Arguments  : uint16 usGpsHour,uint16 usGpsMinute,\
										uint16 usGpsSecond.
			 Will give UTC hour, minute and seconds.
Returns    : uint16. Will give calculated total seconds.
Description: This function is used for calculating the total seconds
			 corresponds to UTC.
*******************************************************************************/
static uint16 TS_TotalSecondsCalc(uint16 usGpsHour,uint16 usGpsMinute,\
										uint16 usGpsSecond)
{
	uint16 usTotal_seconds = 0;

	usGpsHour = (usGpsHour * HOUR_TOTAL_SECONDS);
	usGpsMinute = (usGpsMinute * MINUTE_TOTAL_SECONDS);

    usTotal_seconds = (usGpsHour + usGpsMinute + usGpsSecond);

	return(usTotal_seconds);
}
/******************************************************************************
Name       	:  TS_RunningDelayCalc
Arguments  	:  uint16 delay_calc, total no of seconds.
Returns    	:  total delay that will happen during the run.
Description	:  Will calculate the total delay during the time setting run.
*******************************************************************************/
static uint16 TS_RunningDelayCalc (uint16 usTotalSeconds)
{
	/* variable for delay time calculation */
	/*Precision accuracy is up to count 420. 40=1/.05
	  Means for 20 counts it will take 1 sec(if 50msec is the width) */
	uint16 usTotalRunDelay = 0;

	while(!(usTotalSeconds <= 1))
	 {
		usTotalSeconds =(usTotalSeconds * (RUNNING_DELAY_PARAMETER));  /* 50msec is speed when clock runs fast, should be changed
		 	 	 	 	 	 	 	 	 	 	  	  	  	  	  	when fast clock speed changes*/
		usTotalRunDelay += usTotalSeconds;
	 }

   return(usTotalRunDelay);
}
/******************************************************************************
* Outline 		: TS_DataProcessRMC_GGA
* Description 	: Program for processing the GPS data received.
* Argument  	: GPS array valid, Received array
* Return value  : Total seconds corresponds to UTC time.
******************************************************************************/
static st_GPS_Data TS_DataProcessRMC_GGA(uint8 ucGPSValid,uint8 ucFirstNumber)
{
	int8 ucHour[3]={0};
	int8 ucMinute[3]={0};
	int8 ucSecond[3]={0};
	int8 ucDate[3]={0};
	int8 ucMonth[3]={0};
	int8 ucYear[3]={0};

	int8 cBufferProcess[PACKET_LEN] = {0};
	st_GPS_Data GPSDataStruct;
	DateTime_st stDateStructure;
	uint8 ucDayOfWeek = 0;

	memcpy(cBufferProcess,ucGPSDataReceived,PACKET_LEN);

	/* minute calculation */
	ucMinute[0]=cBufferProcess[ucFirstNumber + MINUTE_FIRST_BYTE]; 	    /* 9the byte */
	ucMinute[1]=cBufferProcess[ucFirstNumber + MINUTE_SECOND_BYTE];     /* 10th byte */
	/* hour calculation */
	ucHour[0]=cBufferProcess[ucFirstNumber + HOUR_FIRST_BYTE];  		/* 7th byte */
	ucHour[1]=cBufferProcess[ucFirstNumber + HOUR_SECOND_BYTE]; 		/* 8th byte */
	/* seconds calculation */
	ucSecond[0]=cBufferProcess[ucFirstNumber + SECOND_FIRST_BYTE];  	/* 11th byte */
	ucSecond[1]=cBufferProcess[ucFirstNumber + SECOND_SECOND_BYTE]; 	/* 12th byte */

	if(ucGPSValid == GPS_VALID)
	{
		/* Date calculation */
		ucDate[0]=cBufferProcess[ucFirstNumber + DAY_FIRST_BYTE_A]; 	/* 53the byte */
		ucDate[1]=cBufferProcess[ucFirstNumber + DAY_SECOND_BYTE_A]; 	/* 54th byte */
		/* Month calculation */
		ucMonth[0]=cBufferProcess[ucFirstNumber + MONTH_FIRST_BYTE_A]; 	/* 55th byte */
		ucMonth[1]=cBufferProcess[ucFirstNumber + MONTH_SECOND_BYTE_A]; /* 56th byte */
		/* Year calculation */
		ucYear[0]=cBufferProcess[ucFirstNumber + YEAR_THIRD_BYTE_A]; 	/* 57th byte */
		ucYear[1]=cBufferProcess[ucFirstNumber + YEAR_FOURTH_BYTE_A]; 	/* 58th byte */
	}
	else if(ucGPSValid == GPS_INVALID)
	{
		/* Date calculation */
		ucDate[0] = cBufferProcess[ucFirstNumber + DAY_FIRST_BYTE_V]; 	/* 25the byte */
		ucDate[1] = cBufferProcess[ucFirstNumber + DAY_SECOND_BYTE_V]; 	/* 26th byte */
		/* Month calculation */
		ucMonth[0]=cBufferProcess[ucFirstNumber + MONTH_FIRST_BYTE_V]; 	/* 27th byte */
		ucMonth[1]=cBufferProcess[ucFirstNumber + MONTH_SECOND_BYTE_V]; /* 28th byte */
		/* Year calculation */
		ucYear[0] = cBufferProcess[ucFirstNumber + YEAR_THIRD_BYTE_V]; 	/* 29th byte */
		ucYear[1] = cBufferProcess[ucFirstNumber + YEAR_FOURTH_BYTE_V]; /* 30th byte */
	}
	else
	{
		GPSDataStruct.ucGPSDataValid = FALSE;
		GPSDataStruct.usTotalGPSSeconds = FALSE;
		return GPSDataStruct;
	}
	/* converting the character values of the time to integer values */
	usHour_data = atoi(ucHour);
	usMinute_data = atoi(ucMinute);
	usSecond_data = atoi(ucSecond);

	usDate_data = atoi(ucDate);
	usMonth_data = atoi(ucMonth);
	usYear_data = (uint16)(CURRENT_CENTURY + atoi(ucYear));

	stDateStructure.ucDay = usDate_data; 						// day of month
	stDateStructure.ucMon = usMonth_data; 						// month
	stDateStructure.usYear = usYear_data; 						// year

	/*Function for returning the day of the week when the date, month and year are given*/
	/*Value will be in between 0 to 6 */
	ucDayOfWeek = TS_GetOddDays( stDateStructure );

	/* If the values are more than the threshold limit return the function */
	if((usHour_data >= HOUR_DAY_MAX) || (usMinute_data >= MINUTE_MAX) || (usSecond_data >= SECOND_MAX) || ucDayOfWeek >= MAX_WEEK_DAY)
	{
		GPSDataStruct.ucGPSDataValid = FALSE;
		GPSDataStruct.usTotalGPSSeconds = FALSE;
		return GPSDataStruct;
	}

	usHour_data += GMT_HOUR;
	usMinute_data += GMT_MINUTE;

	/* when minute data is greater than or equal to 60minutes */
	if(usMinute_data >= MINUTE_MAX)
	{
		usHour_data += 1; 			/* hour when minute exceeds 60 */
		usMinute_data = (usMinute_data - MINUTE_MAX);
	}

	/* when hour data is greater than or equal to 12hours */
	if(usHour_data >= HOUR_MAX)
	{
		/* when hour is greater than or equal to 24 hour data will be */
		if(usHour_data >= HOUR_DAY_MAX)
		{
			usHour_data = (usHour_data - HOUR_DAY_MAX);

			TS_RTC_RegisterSetStart((uint8)usHour_data,(uint8)usMinute_data,(uint8)usSecond_data,ucDayOfWeek);
		}
		else
		{
			TS_RTC_RegisterSetStart((uint8)usHour_data,(uint8)usMinute_data,(uint8)usSecond_data,ucDayOfWeek);

			usHour_data = (usHour_data - HOUR_MAX);
		}
	}
	else
	{
		TS_RTC_RegisterSetStart((uint8)usHour_data,(uint8)usMinute_data,(uint8)usSecond_data,ucDayOfWeek);
	}

	/* Function calling for getting the total seconds corresponds to current IST */
	/* Maximum seconds will be 43199 */
	if((usHour_data < HOUR_MAX) && (usMinute_data < MINUTE_MAX) && (usSecond_data < SECOND_MAX))
	{
		GPSDataStruct.ucGPSDataValid = TRUE;
		GPSDataStruct.usTotalGPSSeconds = TS_TotalSecondsCalc(usHour_data,usMinute_data,usSecond_data);
		return GPSDataStruct;
	}
	else
	{
		GPSDataStruct.ucGPSDataValid = FALSE;
		GPSDataStruct.usTotalGPSSeconds = FALSE;
		return GPSDataStruct;
	}
}
 /******************************************************************************
 * Outline 		: TS_GetOddDays
 * Description 	: Program for getting the day of the week.
 * Argument  	: DateTime_st  stDateMonYear
 * Return value  : Day of the week from 0 to 6.
 ******************************************************************************/
uint8 TS_GetOddDays(DateTime_st  stDateMonYear )
 {
 	unsigned short ucOddDays = ZERO_ODD_DAYS,usTotalDays = 0;
 	uint32 usYear;
 	uint8 ucTotalLeapYear = 0, ucTotalOrdinaryYear = 0, ucMonth;
 	uint8 bIsLeapYear = FALSE,usCounter = 0;
 	/* This array contain the max value of days for all 12 months
 	ignoring the leap year */
 	uint8 ucMaxMonthDay[MAXMONTH] = {31,28,31,30,31,30,31,31,30,31,30,31};

 	bIsLeapYear = IS_LEAP_YEAR( stDateMonYear.usYear );
 	usYear = stDateMonYear.usYear-1;
 	ucMonth = stDateMonYear.ucMon;

 	if( usYear >= ZERO_ODD_DAYS_REF_YEAR_2012 )
 	{
 		/* ZERO_ODD_DAYS is the odd days for the reference year hence,
 		 we are subtracting with reference year, one(1) is added for adjustment*/
 		ucOddDays = ZERO_ODD_DAYS + 1;
 		usYear -= ZERO_ODD_DAYS_REF_YEAR_2012;
 	}
 	else
 	{
 		if( usYear >= ZERO_ODD_DAYS_DURATION_YEARS_400 )
 		{
 			usYear %= ZERO_ODD_DAYS_DURATION_YEARS_400;
 			ucOddDays += ZERO_ODD_DAYS;
 		}

 		if( usYear >= ONE_ODD_DAYS_DURATION_YEARS_300 )
 		{
 			usYear %= ONE_ODD_DAYS_DURATION_YEARS_300;
 			ucOddDays += ONE_ODD_DAYS;
 		}

 		if( usYear >= THREE_ODD_DAYS_DURATION_YEARS_200 )
 		{
 			usYear %= THREE_ODD_DAYS_DURATION_YEARS_200;
 			ucOddDays += THREE_ODD_DAYS;
 		}

 		if( usYear >= FIVE_ODD_DAYS_DURATION_YEARS_100 )
 		{
 			usYear %= FIVE_ODD_DAYS_DURATION_YEARS_100;
 			ucOddDays += FIVE_ODD_DAYS;
 		}
 	}

 	if( usYear >= LEAP_YEAR_DURATION )
 	{
 		ucTotalLeapYear = usYear / LEAP_YEAR_DURATION ;
 	}

 	/* calculate number of odd days upto previous year */
 	ucTotalOrdinaryYear = usYear - ucTotalLeapYear;
 	ucOddDays += ( ucTotalLeapYear << 1 ) + ucTotalOrdinaryYear;

 	/* calculate number of odd days upto current year */
 	/* For leap year making the February days as 29 */
 	if( bIsLeapYear )
 	{
 		ucMaxMonthDay[1] = FEBRUARY_LEAP_DAYS;
 	}

     for(usCounter = 0 ; usCounter < (ucMonth - 1) ; usCounter++)
 	{
 		usTotalDays += ucMaxMonthDay[usCounter];
 	}

 	usTotalDays += stDateMonYear.ucDay;

 	ucOddDays = ( ucOddDays + usTotalDays ) % MAX_WEEK_DAY;

 	ucMaxMonthDay[1] = FEBRUARY_NORMAL_DAYS;	    // re initializing February month days.

 	return ucOddDays;
 }

 /**************************************************************************//**
 * TS_RXDelayWaitInit
 * @brief         - Program for creating the second delay.
 * @param         - input the delay in seconds
 * @return        -
 ******************************************************************************/
 uint8 TS_RXDelayWaitStart (unsigned short usDelaysec, uint8 ucRXDelayMode)
 {
	 if( usDelaysec < 1 )
	 {
		return FAILURE;
	 }
	 ucRXDelayModeSelect = ucRXDelayMode;
	 usDelaysecs = usDelaysec;
	 usDelaysecCount = 0;

	 TA0CCR0    = (RXTIMER_COUNT - 1);    	    // x cycles * 1/4096 = y us
	 TA0CCTL0  |= CCIE;
	 TA0CTL    |= MC_1 + TACLR ;  				// Start the timer

	 return SUCCESS;
 }
 /**************************************************************************//**
 * TS_RXDelayWaitInit
 * @brief         - program for initializing the timer registers
 * @param         - none
 * @return        - none
 ******************************************************************************/
 void TS_RXDelayWaitInit (void)
 {
	 TA0R    = 0;								 // clear the counter
	 TA0CTL &= ~(TAIFG + TAIE);				 	 // clearing all the int flags and interrupts
	 TA0CTL |= TASSEL__ACLK + ID__8 + TACLR ; 	 // select the clock source and pre scalar
 }
 /**************************************************************************//**
 * TS_RXDelayWaitStop
 * @brief         - Program for stopping the timer.
 * @param         - none
 * @return        - none
 ******************************************************************************/
 void TS_RXDelayWaitStop (void)
 {
	 TA0CTL &= ~MC_3;							// disable the timer
	 TA0R    = 0;								// clear the counter
	 TA0CTL &= ~(TAIFG + TAIE);					// disable the int and flags
 }

 /**************************************
 * Interrupt Service Routine for TIMER0
 **************************************/
 #pragma vector=TIMER0_A0_VECTOR
 __interrupt void TIMER0_A0_ISR(void)
 {
  TA0R = 0;								// clear the counter;

  if(ucRXDelayModeSelect == RX_ON_MODE)
  {
	  usDelaysecCount++;
	  if( usDelaysecCount == usDelaysecs )
	  {
		  ucGPSLinkLossCheck++;				// counter for total number of ONs
		  usDelaysecCount = 0;
		  usDelaysecs = 0;
		  ucRXDelayModeSelect = 0;

		  TS_RXDelayWaitStop();
		  __bic_SR_register_on_exit(LPM3_bits);
	  }
  }
  if(ucRXDelayModeSelect == RX_OFF_MODE)
  {
	  usDelaysecCount++;
	  if( usDelaysecCount == usDelaysecs )
	  {
		  usDelaysecCount = 0;
		  usDelaysecs = 0;
		  ucRXDelayModeSelect = 0;

		  TS_RXDelayWaitStop();
		  __bic_SR_register_on_exit(LPM3_bits);
	  }
  }
 }

 /**************************************
 * Interrupt Service Routine for Radio
 **************************************/
 #pragma vector=CC1101_VECTOR
 __interrupt void CC1101_ISR(void)
 {
   static uint8 RxBufferLength = 0;			  // value should be 63(send packet length)

   receiving =((receiving & TX_IN_RANGE_MASK) | TX_IN_RANGE_BYTE);

   switch(__even_in_range(RF1AIV,32))        // Prioritizing Radio Core Interrupt
   {
     case  0: break;                         // No RF core interrupt pending
     case  2: break;                         // RFIFG0
     case  4: break;                         // RFIFG1
     case  6: break;                         // RFIFG2
     case  8: break;                         // RFIFG3
     case 10: break;                         // RFIFG4
     case 12: break;                         // RFIFG5
     case 14: break;                         // RFIFG6
     case 16: break;                         // RFIFG7
     case 18: break;                         // RFIFG8
     case 20:                                // RFIFG9
		   /* interrupt occurs at the end of packet */
		   /* 1 means high to low transmission*/
		   if(!(RF1AIN & BIT9))				//if(!(RF1AIES & BIT9))
		   {
				/* Read the length byte from the FIFO */
				RxBufferLength = ReadSingleReg(RXBYTES);
				/*Mask the bytes so that only the bytes number will get*/
				RxBufferLength &= RX_BYTES_MASK;

				if(RxBufferLength == PACKET_LEN)
				{
					/* variable set indicating valid data received. */
					receiving =((receiving & RAD_RECIEVED_MASK) | RAD_RECIEVED_BYTE);

					ReadBurstReg(RF_RXFIFORD,ucGPSDataReceived,RxBufferLength);
					// Stop here to see contents of RxBuffer
					__no_operation();

					RxBufferLength = 0;
					TS_RXDelayWaitStop();				 	// making the delay timer off.

					__bic_SR_register_on_exit(LPM3_bits);   // Exit active
				}
				else
				{
					  RF1AIFG = 0;           				// Clear pending IFG
					  RxBufferLength = 0;
				}
		   }
		   else
		   {
			   RF1AIFG = 0;           						// Clear pending IFG
			   __no_operation();
		   }
		   break;
     case 22: break;                         // RFIFG10
     case 24: break;                         // RFIFG11
     case 26: break;                         // RFIFG12
     case 28: break;                         // RFIFG13
     case 30: break;                         // RFIFG14
     case 32: break;                         // RFIFG15
   }
 }

