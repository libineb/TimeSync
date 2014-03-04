/*
 * TS_Timer_Pwms.c
 *
 *  Created on: Mar 21, 2013
 *      Author: libin.eb
 */
//Timer_A3, PWM TA1.1-2, Up Mode, 32kHz ACLK
//  ACLK = TACLK = LFXT1 = 32768Hz,
//
//                CC430F5137
//            -------------------
//        /|\|                   |
//         | |                   |
//         --|RST                |
//           |                   |
//           |         P2.0/TA1.1|--> CCR1 - 1st PWM
//           |         P2.2/TA1.2|--> CCR2 - 2nd PWM
/************************************************************/
/* Header files */
#include "TS_Timer_Pwms.h"

/*variables */
TS_PWMmodes ucPwmMode;
static uint8 ucToggleDutyCycle = FALSE;

static uint16  usClockSecondsCount = FALSE;			// normal movement tracking for check
static uint16  usClockFastSecondsCount = FALSE;		// fast movement tracking
static uint16  usClockAheadSecondsCount = FALSE;	// ahead movement tracking

static uint8 ucMinuteSensorStartDelay = 0;			// minute sensor start delay flag
static uint16 usMinuteSenserDelayCount = 0;

/* Local functions */
static void TS_TimerPwm_Start(void);
static uint8 TS_TimerRegisterSettings(TS_PWMmodes RegisterSelection);

/******************************************************************************
* Outline 		: TS_TimerPwm_Init
* Description 	: Program for Initialization of Timer TA1 CCR1 and CCR2
* 				  output from P2.0 and P2.2.
* Argument  	: none
* Return value  : none
******************************************************************************/
void TS_TimerPwm_Init(void)
{
  PMAPPWD = 0x02D52;           // Get write-access to port mapping regs
  P2MAP0 = PM_TA1CCR1A;        // Map TA1CCR1 output to P2.0
  P2MAP2 = PM_TA1CCR2A;        // Map TA1CCR2 output to P2.2  Default mapping
  PMAPPWD = 0;                 // Lock port mapping registers

  P2DIR |= PWM_COIL1 + PWM_COIL2;  //P2.0=CCR1,P2.2=CCR2
  P2SEL |= PWM_COIL1 + PWM_COIL2;  // P2.0 and P2.2 peripherals select.
}
/******************************************************************************
* Outline 		: TS_TimerPwm_Start
* Description 	: Program for starting the timer and going into LPM3.
* Argument  	: none
* Return value  : none
******************************************************************************/
static void TS_TimerPwm_Start(void)
{
	TA1CCTL0 |= CCIE;
	TA1CTL   |= (TASSEL_1 + MC_1 + TACLR);  // ACLK, Timer A mode-Up to CCR0,FALSE TAR

	__bis_SR_register(LPM3_bits); 		 	// Enter LPM3 & Enable interrupts, GIE is already enabled.
}
/******************************************************************************
* Outline 		: TS_TimerPwm_Stop
* Description 	: Program for stopping the timer.
* Argument  	: none
* Return value  : none
******************************************************************************/
void TS_TimerPwm_Stop(void)
{
	TA1CCTL0 &= ~(CCIE);     // CCR0 interrupt disabled
	TA1CTL   &= ~(MC_3);   	 // Timer A mode control: 0 - Stop
	TA1R      = 0; 			 // clearing the timer counter
}
/******************************************************************************
* Outline 		: TS_TimerRegisterSettings
* Description 	: Program for selecting the timer register values.
* Argument  	: enum for determine the values of registers
* Return value  : unsigned char for Selecting weather Timer should start or not
******************************************************************************/
static uint8 TS_TimerRegisterSettings(TS_PWMmodes RegisterSelection)
{
	uint8 ucPwmStart = OFF;

	ucPwmMode = RegisterSelection;
	switch (RegisterSelection)
	{
	case TS_NormalMode :TA1CCR0 = CCR0_ONE_SEC_PERIOD;
						TA1CCTL1 |= OUTMOD_3;	    // Set/Reset
						TA1CCR1 = CCR1_FIRST_DUTYCYCLE;
						TA1CCTL2 |= OUTMOD_3;
						TA1CCR2 = CCR2_SECOND_OFF;

						RED_LED_OFF; 				// make all leds OFF
						GREEN_LED_OFF;

						ucPwmStart = ON;
//						TS_RESETSwitchInit();		//Enable RESET switch, not needed now
						break;
	  case TS_FastMode :TA1CCR0 = (uint16)CCR0_FAST_SEC_PERIOD;
						TA1CCTL1 |= OUTMOD_3;
						TA1CCR1 = CCR1_FIRST_DUTYCYCLE_FAST;
						TA1CCTL2 |= OUTMOD_3;
						TA1CCR2 = CCR2_FAST_OFF;

						ucPwmStart = ON;
//						TS_RESETSwitchInit();		// Enable RESET switch, not needed now
						break;
case TS_InitialSetMode :
						TA1CCR0 = (uint16)CCR0_FAST_SEC_PERIOD;
						TA1CCTL1 |= OUTMOD_3;
						TA1CCR1 = CCR1_FIRST_DUTYCYCLE_FAST;
						TA1CCTL2 |= OUTMOD_3;
						TA1CCR2 = CCR2_FAST_OFF;

						ucPwmStart = ON;
//						TS_HourSecondSensor_Start(SECOND_HAND,INTERRUPT_ENABLE); 	// Start Second sensor when second hand sensing need
						TS_MinuteSensorStart(INTERRUPT_ENABLE);
						break;
	 case TS_AheadMode :TA1CCR0 = CCR0_ONE_SEC_PERIOD;
						TA1CCTL1 &= ~OUTMOD_3;
						TA1CCR1 = CCR1_SECOND_OFF;
						TA1CCTL2 &= ~OUTMOD_3;
						TA1CCR2 = CCR2_SECOND_OFF;

						ucPwmStart = ON;
						break;
	   case TS_OFFMode :TA1CCR0 = OFF;
						TA1CCTL1 &= ~OUTMOD_3;
						TA1CCR1 = OFF;
						TA1CCTL2 &= ~OUTMOD_3;
						TA1CCR2 = OFF;

						TS_TimerPwm_Stop();
						TS_Sensor_Stop();

						ucFirstInterruptPin = FALSE;
						ucSecondInterruptPin = FALSE;
						break;
			   default: break;
	}

    return ucPwmStart;
}
/******************************************************************************
* Outline 		: TS_TimerPwm_Mode
* Description 	: Program for generating PWMSs for normal and fast mode and initial
* 				  setting. also making the PWM module OFF.
* Argument  	: enum for selecting the mode of operation.
* Return value  : none
******************************************************************************/
void TS_TimerPwm_Mode(TS_PWMmodes ucModeSelection)
{
	uint8 ucTimerstart = 0;
	uint16 usProxiReadCheck = 0;

	ucToggleDutyCycle = TRUE;			// setting the flag for changing the duty cycle.
    ucMinuteSensorStartDelay = OFF;		// clearing the minute sensor start on flag
    ucFirstInterruptPin = FALSE;		// Clearing the sensor interrupts flags.
    ucSecondInterruptPin = FALSE;

	ucTimerstart = TS_TimerRegisterSettings(ucModeSelection);	// configuring the timer registers

	/* For continuous operation check */
    while(ucTimerstart == ON)
    {
      /* PWM timer start. when any interrupt occurs it returns back here */
	  TS_TimerPwm_Start();

	  switch(ucPwmMode)
	  {
	   case TS_NormalMode:
						  /* RTC interrupt occurred */
						  if(TS_RTCReturnIntFlag() == TRUE)
						  {
							  ucFirstInterruptPin = FALSE;
							  ucSecondInterruptPin = FALSE;

							  TS_RTCClearIntFlag();
							  TS_RTC_ReadStop();

							  ucTimerstart = OFF;
						  }
						  /* Any other fault condition occurs */
						  else
						  {
							 TS_TimerPwm_Stop();
							 TS_Sensor_Stop();

							 ucFirstInterruptPin = FALSE;
							 ucSecondInterruptPin = FALSE;
							 ucTimerstart = OFF;
						  }
						  break;
		   case TS_FastMode :
			   	   	   	   	  /* Clearing the flags */
							  ucFirstInterruptPin = FALSE;
							  ucSecondInterruptPin = FALSE;
							  ucTimerstart = OFF;
							  break;
	 case TS_InitialSetMode :

						 if(ucSecondInterruptPin == TRUE)
						 {
							/* should include when second hand sensing will come */
#if 0
						   ucSecondInterruptPin = FALSE;
						   usProxiReadCheck = FALSE;

						   usProxiReadCheck = TS_ReturnProximityReadData();
						   if(usProxiReadCheck >= 0x0200)		// Hour hand detected. *Bug. 0x01D8
						   {
							   /*Interrupt cleared and sensor stopped*/
							  TS_Sensor_Stop();

							  /*Exit while loop*/
							  ucTimerstart = OFF;
						   }
						   else if( (0x0080 < usProxiReadCheck) && (usProxiReadCheck < 0x0139) ) // Minute hand detected.
						   {
							   /*Interrupt cleared and sensor stopped*/
							  TS_Sensor_Stop();

							  /*Exit while loop*/
							  ucTimerstart = OFF;
						   }
						   else										// Second hand detected.
						   {
							  TS_Sensor_Stop();

							  /*Speed up the clock movement*/
							  TA1CCR0=CCR0_FAST_SEC_PERIOD;

							  /*Second hand detected,now sense for minute hand*/
							  TS_MinuteSensorStart(INTERRUPT_ENABLE);
						   }
#endif
						 }
						 else if(ucFirstInterruptPin == TRUE)
						  {
							  ucFirstInterruptPin = FALSE;		// variable should be FALSE after stooping the sensor
							  usProxiReadCheck = FALSE;

							  GREEN_LED_ON; 					// check

							  /*Stop the minute sensor*/
							  TS_Sensor_Stop();
							  /*Start the hour sensor*/
							  TS_HourSecondSensor_Start(HOUR_HAND,INTERRUPT_DISABLE);
//							  __delay_cycles(2958);				/* Delay __delay_cycles(8051);	*/

							  usProxiReadCheck = TS_ReturnProximityReadData();
							  __no_operation();
							  /* 12 o clock position detected */
							  if((usProxiReadCheck >= HOUR_THRESHOLD_VALUE) || (ucSecondInterruptPin == TRUE))
							  {
								  RED_LED_ON;
								  GREEN_LED_ON;

								  TS_Sensor_Stop();

								  ucSecondInterruptPin = FALSE;
								  ucMinuteSensorStartDelay = OFF;
								  ucTimerstart = OFF;
							  }
							  else
							  {
								  /* Making the flag set for giving the minute sensor start delay */
								  ucMinuteSensorStartDelay = ON;
								  TS_Sensor_Stop();
							  }
						  }
						  else if(usMinuteSenserDelayCount == SECONDS_OF_FIRSTSENSOR_START)
						  {
							  usMinuteSenserDelayCount = FALSE;

							  /*after the minute sensor start delay expires the minute sensor start again*/
							  TS_MinuteSensorStart(INTERRUPT_ENABLE);
						  }
						  else
						  {
							__no_operation();                 // For debugger
						  }
						  break;
	 case TS_AheadMode	: ucFirstInterruptPin = FALSE;
						  ucSecondInterruptPin = FALSE;
						  ucTimerstart=OFF;
						  break;
				 default: break;
	  }
   }
}
/**************************************
 Interrupt Service Routines
**************************************/
// Timer A0 interrupt service routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
	/* clearing the timer counter */
	TA1R = 0;
	switch(ucPwmMode)
	{
		case TS_NormalMode:
							if(!ucToggleDutyCycle)
							{
								ucToggleDutyCycle ^= 1;

								TA1CCR1 = CCR1_FIRST_DUTYCYCLE;             // CCR1 PWM duty cycle
								TA1CCR2 = CCR2_SECOND_OFF;                 	// CCR2 PWM duty cycle
							}
							else
							{
								ucToggleDutyCycle ^= 1;

								TA1CCR1 = CCR1_SECOND_OFF;				  	// CCR1 PWM duty cycle
								TA1CCR2 = CCR2_FIRST_DUTYCYCLE;             // CCR2 PWM duty cycle
							}
						   break;
	     case TS_FastMode:
							usClockFastSecondsCount++;
							if(usClockFastSecondsCount == (TS_ReturnActualSecondsToRotate() + 1))
							{
								usClockFastSecondsCount = FALSE;

								TS_TimerPwm_Stop();
								__bic_SR_register_on_exit(LPM3_bits);
							}
							if(!ucToggleDutyCycle)
							{
								ucToggleDutyCycle ^= 1;

								TA1CCR1 = CCR1_FIRST_DUTYCYCLE_FAST;
								TA1CCR2 = CCR2_FAST_OFF;
							}
							else
							{
								ucToggleDutyCycle ^= 1;

								TA1CCR1 = CCR1_FAST_OFF;
								TA1CCR2 = CCR2_FIRST_DUTYCYCLE_FAST;
							}
						   break;
	case TS_InitialSetMode :
							if(ucMinuteSensorStartDelay == ON)
							{
								usMinuteSenserDelayCount++;
								/* Make the minute sensor start after 55 minutes */
								if(usMinuteSenserDelayCount == SECONDS_OF_FIRSTSENSOR_START)
								{
									ucMinuteSensorStartDelay = OFF;

									TS_TimerPwm_Stop();
									__bic_SR_register_on_exit(LPM3_bits);
									break;
								}
							}
							if(!ucToggleDutyCycle)
							{
								ucToggleDutyCycle ^= 1;

								TA1CCR1 = CCR1_FIRST_DUTYCYCLE_FAST;
								TA1CCR2 = CCR2_FAST_OFF;
							}
							else
							{
								ucToggleDutyCycle ^= 1;

								TA1CCR1 = CCR1_FAST_OFF;
								TA1CCR2 = CCR2_FIRST_DUTYCYCLE_FAST;
							}
						   break;
		case TS_AheadMode:
							if(usClockAheadSecondsCount == (TS_ReturnActualSecondsToRotate()))
							{
								TS_TimerPwm_Stop();
								__bic_SR_register_on_exit(LPM3_bits);
							}
				default :break;
	}
}


