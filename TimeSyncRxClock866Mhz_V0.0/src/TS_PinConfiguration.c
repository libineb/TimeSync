/*
 * TS_PinConfiguration.c
 *
 *  Created on: May 9, 2013
 *      Author: libin.eb
 */
/* Header files */
#include "TS_PinConfiguration.h"

/*extern variables initializations*/
volatile uint8 ucFirstInterruptPin = 0;
volatile uint8 ucSecondInterruptPin = 0;

/**************************************************************************//**
* TS_LEDInit
* @brief         - Program for Initializing the LED Pins
* @param         -
* @return        -
******************************************************************************/
void TS_LEDInit(void)
{
  P1OUT &= ~GREEN_LED_PIN;			  /* Init for GREEN LED */
  P1DIR |= GREEN_LED_PIN;
  P3OUT &= ~RED_LED_PIN;			  /* Init for RED LED */
  P3DIR |= RED_LED_PIN;
}

/**************************************
* Interrupt Service Routines
**************************************/
/* For PORT1, Switch and Sensor Interrupt */
#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
  switch(__even_in_range(P1IV, 16))
  {
    case  0: break;
    case  2: break;                         // P1.0 IFG
    case  4: break;                         // P1.1 IFG
    case  6: break;                         // P1.2 IFG
    case  8: break;                         // P1.3 IFG
    case 10:
    	     /* First Sensor interrupt came  P1.4 IFG */
		 	 TS_TimerPwm_Stop();

    		 ucFirstInterruptPin = TRUE;	// flag set
			 MINUTE_INT_IE  &= ~(MINUTE_INT); // disable the interrupt
			 MINUTE_INT_IFG  = 0;			// clear the flags

			 __bic_SR_register_on_exit(LPM3_bits);
    		 break;
    case 12: break;                         // P1.5 IFG
    case 14: break;                         // P1.6 IFG
    case 16: break;
    default : break;
  }
}

/* PORT2 Sensor interrupt ISR */
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
  switch(__even_in_range(P2IV, 16))
  {
    case  0: break;
    case  2: break;                         // P2.0 IFG
    case  4: break;                         // P2.1 IFG
    case  6: break;                         // P2.2 IFG
    case  8: break;                         // P2.3 IFG
    case 10:
	        /* Second Sensor interrupt came  P2.4 IFG */
		 	 TS_TimerPwm_Stop();

    		 ucSecondInterruptPin=TRUE;		// flag set
    		 HOUR_INT_IE &= ~(HOUR_INT);	// disabling the second sensor interrupt
    		 HOUR_INT_IFG = 0;				// flag cleared.

			 __no_operation();
			 __bic_SR_register_on_exit(LPM3_bits);
			 break;

    case 12: break;                         // P2.5 IFG
    case 14: break;                         // P2.6 IFG
    case 16: break;                         // P2.7 IFG
    default: break;
  }
}
/* ISR for low power indication */
/* For SYSTEM NMI ISR for low power indication */
#pragma vector=SYSNMI_VECTOR
__interrupt void SYSNMI_ISR(void)
{
  switch(__even_in_range(SYSSNIV,20))
  {
	case 0	: break;						// No interrupt
	case 2	: break;						// SVMLIFG interrupt
	case 4	:
			  // SVMHIFG interrupt
			  GREEN_LED_OFF;				// making all the leds OFF
			  RED_LED_OFF;
			  /* No need. In the SVM module enabling the SVM pin as active low */
			  RED_LED_ON;					 // for check

			  TS_TimerPwm_Stop();			 // Stop clock
//			  TS_TimerPwm_Mode(TS_OFFMode);	 // for check disabled
			  ReceiveOff();					 // for check disabled
			  ResetRadioCore();

			  while(1);						 // while loop trap showing the low power
			  break;
	case 6	: break;						// SVSMLDLYIFG interrupt
	case 8	: break;						// SVSMHDLYIFG interrupt
	case 10 : break;						// VMAIFG interrupt
	case 12 : break;						// JMBINIFG interrupt
	case 14 : break;						// JMBOUTIFG interrupt
	case 16 : break;						// SVMLVLRIFG interrupt
	case 18 :								// SVMHVLRIFG interrupt
			  break;
	case 20 : break;						// Reserved
	default	: break;
  }
}


