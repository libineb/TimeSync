/*
 * TS_HardwareInit.c
 *
 *  Created on: Apr 18, 2013
 *      Author: libin.eb
 */
/* User defined header file */
#include "TS_Hardware_Init.h"

/* Local functions */
static void TS_ClockFrequencyInit(void);
static void TS_SVMDetection(void);

/**************************************************************************//**
* TS_HardwareInit
* @brief         - All the hardware initializations.
* @param         -
* @return        -
******************************************************************************/
void TS_HardwareInit(void)
{
#if 0
	  P1OUT = 0x00; P2OUT = 0x00; P3OUT = 0x00; P5OUT = 0x00;
	  P1DIR = 0xFF; P2DIR = 0xFF; P3DIR = 0xFF; P5DIR = 0x03;
#endif
	/* Initializing the clock */
	TS_ClockFrequencyInit();
	/* Initializing the led */
	TS_LEDInit();
	/*Low voltage detection*/
	TS_SVMDetection();
	/* Initialization of timer PWMS */
	TS_TimerPwm_Init();

	__bis_SR_register(GIE);					// enabling general interrupts
	__no_operation();
}
/**************************************************************************//**
* TS_ClockFrequencyInit
* @brief         - Program for selecting the 32KHz external crystal and
* 				   changing the main clock of the system to 0.98Mhz.
* 				   Also unlock the port mapping registers.
* @param         -
* @return        -
******************************************************************************/
static void TS_ClockFrequencyInit(void)
{
	 /* selecting pins for enabling the 32KHz external crystal*/
	 P5SEL |= BIT1 + BIT0;                  	// Set xtal pins peripheral
	 UCSCTL6 |= XCAP_3;                        	// Internal load cap

	  do
	  {
	    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);  // Clear XT2,XT1,DCO fault flags
	    SFRIFG1 &= ~OFIFG;                      // Clear fault flags
	  }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

	 /*Initialize DCO to 0.98MHz, When PUC happens MCLK is from DCOCLKDIV by default*/
	  __bis_SR_register(SCG0);                  // Disable the FLL control loop
	  UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
	  UCSCTL1 = DCORSEL_1;                      // Set RSELx for DCO = 1.96MHz,if 10= 0.72Mhz
	  UCSCTL2 = FLLD_1 + 10;                    // Set DCO Multiplier for 0.98MHz
	                                            // (N + 1) * FLLRef = Fdco
	                                            // (10 + 1) * 32768 = 0.98MHz, if 10= 0.36Mhz
	                                            // Set FLL Div = fDCOCLK/2 // by default DCOCLK division is 2
	  __bic_SR_register(SCG0);                  // Enable the FLL control loop

	  // Worst-case settling time for the DCO when the DCO range bits have been
	  // changed is n x 32 x 32 x f_MCLK / f_FLL_reference.
	  // 32 x 32 x 0.98MHz / 32,768 Hz = 30720 = MCLK cycles for DCO to settle
	  __delay_cycles(11250);

	  // Loop until XT1,XT2 & DCO fault flag is cleared
	  do
	  {
	    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
	                                            // Clear XT2,XT1,DCO fault flags
	    SFRIFG1 &= ~OFIFG;                      // Clear fault flags
	  }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

	 /* making the port mapping registers access at any time*/
	 PMAPPWD = 0x02D52;           				// Get write-access to port mapping regs
	 PMAPCTL = PMAPRECFG;		   				// Reconfiguring the PORT mapping registers
	 PMAPPWD = 0;                 				// Lock port mapping registers
}
/******************************************************************************
* Outline 		: TS_SVMDetection
* Description 	: Program for supply voltage supervisor and low voltage indicator.
* Argument  	: none
* Return value  : none
******************************************************************************/
static void TS_SVMDetection(void)
{
	PMAPPWD = 0x02D52;           				// Get write-access to port mapping regs
	P2MAP5 	= PM_SVMOUT;        				// Map PM_SVMOUT output to P2.5
	PMAPPWD = 0;                 				// Lock port mapping registers

	P2DIR |= LOW_POWER_LED;        				// setting the peripheral
	P2SEL |= LOW_POWER_LED;

	PMMCTL0_H = 0xA5;							// Open PMM registers for write access.

	/*SVM high-side enable.
	 SVM and SVS manual control core level is 2 so minimum level 3 can be given, maximum level 5.

	 SVSMHRRL_5 (2.52 2.65 2.78 voltages) voltage increase, at low voltage clock rotation
	 weakens.
	 2.10 2.20 2.30(SVSMHRRL_3)
	 2.25 2.35 2.50(SVSMHRRL_4)
	 2.52 2.65 2.78(SVSMHRRL_5)
	 */
	SVSMHCTL = SVMHE + SVSMHRRL_3 + SVSHMD;
	while ((PMMIFG & SVSMHDLYIFG) == 0);		// Wait until SVM high side is settled

	/* SVMOUT pin polarity. If this bit is set, SVMOUT is active high.
	   This will glow a RED LED.*/
	SVSMIO &= ~SVMOUTPOL;						// output is set as active low(In the schematic the pin is active low).
	SVSMIO |= SVMHVLROE;						// SVM high-side voltage level reached output enable.

	PMMRIE |=(SVMHIE);							// SVM high side interrupt enable

  	PMMCTL0_H = 0x00;							// Lock PMM registers for write access
}

