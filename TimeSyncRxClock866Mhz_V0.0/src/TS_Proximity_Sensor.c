/*
 * TS_Proximity_Sensor.c
 *
 *  Created on: May 3, 2013
 *      Author: libin.eb
 */
//******************************************************************************
//
//                                /|\  		/|\
//                   TMD2721      1.5k 		 1.5k     CC430F5137
//                   slave         |   		 |        master
//             -----------------   |    	 |----------------- 		----------------|
//           -|		  			|<-|------+->|P1.3/UCB0SDA     |		|				|
//            |                 |  |   	     |     P2.3/UCB0SDA|<-|--+->|				|
//           -|	                |  |   	     |           	   |		|				|
//            |     		    |<-+-------->|P1.2/UCB0SCL     |		|				|
//            |                 |            |             	   |		|				|
//            |                 |            |        P2.6/SCL |<-+---->|				|
//            |                 |<-+-------->|P1.4/INT1        |        |				|
//            |                 |            |       P2.4/INT1 |        |				|
//            |                 |            |             	   |		|				|
//***************************************************************************************
#include "TS_Proximity_Sensor.h"

/*Commands for setting the registers in the sensor*/
const unsigned char ucMinuteSensorStart[TOTAL_COMMANDS_MINUTE][TOTAL_BYTES_SEND]=
	{
		{COMMAND_PULSE_COUNT_REG,PULSE_COUNT_MINUTE},/*{COMMAND_CONFIG_REG,CONFIG_VALUE},
		{COMMAND_WAIT_TIME_REG,WAIT_TIME_VALUE},*/
		{COMMAND_CONTROL_REG,DIODE_GAIN_MINUTE},{COMMAND_PERSISTENCE_REG,PERSISTANCE_VALUE},
		{COMMAND_HIGH_LOW_BYTE_REG,INT_HIGH_LOW_MINUTE},{COMMAND_HIGH_HIGH_BYTE_REG,INT_HIGH_HIGH_MINUTE}
//		{COMMAND_ENABLE_REG,PON_INT_PROX_ENABLE}		//PON_INT_PROX_SLEEP_ENABLE
	};
const unsigned char ucHourSensorStart[TOTAL_COMMANDS_HOUR][TOTAL_BYTES_SEND]=
	{
		{COMMAND_PULSE_COUNT_REG,PULSE_COUNT_HOUR},/*{COMMAND_CONFIG_REG,CONFIG_VALUE},
		{COMMAND_WAIT_TIME_REG,WAIT_TIME_VALUE},*/
		{COMMAND_CONTROL_REG,DIODE_GAIN_HOUR},{COMMAND_PERSISTENCE_REG,PERSISTANCE_VALUE},
		{COMMAND_HIGH_LOW_BYTE_REG,INT_HIGH_LOW_HOUR},{COMMAND_HIGH_HIGH_BYTE_REG,INT_HIGH_HIGH_HOUR}
//		{COMMAND_ENABLE_REG,PON_PROX_ENABLE}  // Disabling the interrupt only polling for the hour hand
	};
const unsigned char ucSecondStart[TOTAL_COMMANDS_SECOND][TOTAL_BYTES_SEND]=
	{
		{COMMAND_PULSE_COUNT_REG,PULSE_COUNT_SECOND},/*{COMMAND_CONFIG_REG,CONFIG_VALUE},*/
		{COMMAND_WAIT_TIME_REG,WAIT_TIME_SECOND},
		{COMMAND_CONTROL_REG,DIODE_GAIN_SECOND},{COMMAND_PERSISTENCE_REG,PERSISTANCE_VALUE},
//		{COMMAND_LOW_LOW_BYTE_REG,INT_LOW_HIGH_SECOND},{COMMAND_LOW_HIGH_BYTE_REG,INT_LOW_LOW_SECOND},
		{COMMAND_HIGH_LOW_BYTE_REG,INT_HIGH_LOW_SECOND},{COMMAND_HIGH_HIGH_BYTE_REG,INT_HIGH_HIGH_SECOND}
//		{COMMAND_ENABLE_REG,PON_INT_PROX_ENABLE}  // Edited for disabling the interrupts
	};

/*constant for clearing the interrupt*/
const unsigned char BufferInterruptClear = COMMAND_INT_CLEAR_SPECIAL;
/*constant for clearing the enable register*/
const unsigned char ucBufferProxiDisabled[TOTAL_BYTES_SEND] = {COMMAND_ENABLE_REG,0x00};
/* command for starting the proximity with interrupt enabled */
const unsigned char ucProxiStartWithInt[TOTAL_BYTES_SEND] = {COMMAND_ENABLE_REG,PON_INT_PROX_WAIT_ENABLE};
/* command for starting the proximity with interrupt enabled */
const unsigned char ucProxiStartWithOutInt[TOTAL_BYTES_SEND]={COMMAND_ENABLE_REG,PON_PROX_ENABLE};
/*Register command for reading proximity data value*/
const unsigned char ucProxiRead = 0xB8;

const unsigned char *PTxData;              // Pointer to TX data
static uint8 TXByteCtr;
/*variables for receive*/
static uint8 *PRxData;                     	// Pointer to RX data
static uint8 RXByteCtr;

/**************************************************************************//**
* TS_MinuteSensorInterrupt_Init
* @brief         - First Interrupt pin initialization(P1.4).
* @param         -
* @return        -
******************************************************************************/
static void TS_MinuteSensorInterrupt_Init (void)
{
	 /*Disabling the hour interrupt*/
	  HOUR_INT_DIR |= HOUR_INT;
	  HOUR_INT_OUT |= HOUR_INT;

	  MINUTE_INT_DIR &= ~MINUTE_INT;	// as input
	  MINUTE_INT_REN |= MINUTE_INT;		// Pull up or pull down enabled.
	  MINUTE_INT_IES |= MINUTE_INT;		// P1IFG flag is set with a high-to-low transition.
	  MINUTE_INT_OUT |= MINUTE_INT;		// Pull up selected
	  MINUTE_INT_IFG  = 0;				// No interrupt is pending
	  MINUTE_INT_IE  |= MINUTE_INT;		// interrupt enabled
}
/**************************************************************************//**
* TS_SecondSensorInterrupt_Init
* @brief         - Second Interrupt pin initialization(P2.4).
* @param         -
* @return        -
******************************************************************************/
static void TS_HourSecondSensorInterrupt_Init (void)
{
	 /*Disabling the minute interrupt*/
	  MINUTE_INT_DIR |= MINUTE_INT;
	  MINUTE_INT_OUT |= MINUTE_INT;

	  HOUR_INT_DIR &= ~HOUR_INT;
	  HOUR_INT_REN |= HOUR_INT;
	  HOUR_INT_IES |= HOUR_INT;
	  HOUR_INT_OUT |= HOUR_INT;		    		// 10K Ohm pull up resistor used.
	  HOUR_INT_IFG  = 0;
	  HOUR_INT_IE  |= HOUR_INT;
}
/**************************************************************************//**
* TS_SensorIIC_Start
* @brief         - All IIC register initializations.
* @param         -
* @return        -
******************************************************************************/
static void TS_SensorIIC_Start (void)
{
	  UCB0CTL1 |= UCSWRST;                      // Enable SW reset

	  UCB0CTL0 = (UCMST + UCMODE_3 + UCSYNC);   // I2C mode, Master, synchronous mode
	  UCB0CTL1 = (UCSSEL__ACLK + UCSWRST);      // Use ACLK=32Khz, keep SW reset
	  UCB0BR0  = 12;                            // fSCL = ACLK/12 = ~2.73kHz  //183.1uSec period
	  UCB0BR1  = 0;								// pre-scaler value = (UCxxBR0 + UCxxBR1 × 256)
	  UCB0I2CSA = SLAVE_ADDRESS;                // Slave Address for sensor.

	  UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation

	  UCB0IE |=  UCALIE;   						// Enable TX interrupt, RX interrupt
}
/******************************************************************************
* Outline 		: TS_MinuteSensor_Init
* Description 	: IIC hardware Initialization for first sensor.
* Argument  	: none
* Return value  : none
******************************************************************************/
static void TS_MinuteSensor_Init (void)
{
	  TS_MinuteSensorInterrupt_Init();

	  PMAPPWD = PMAPKEY;                        // Get write-access to port mapping registers.
	  P1MAP3  = PM_UCB0SDA;                     // P1.3 // I2C data
	  P1MAP2  = PM_UCB0SCL;                     // P1.2 // I2C clock
	  P2MAP6  = PM_NONE;                     	// PM_NONE output to P2.6
	  P2MAP3  = PM_NONE;                     	// PM_NONE output to P2.3
	  PMAPPWD = 0;                              // Lock port mapping registers

	  MINUTE_IIC_SEL |= MINUTE_SCL + MINUTE_SDA;
	  HOUR_IIC_SEL   &= ~(HOUR_SDA + HOUR_SCL);

	  TS_SensorIIC_Start();
}
/******************************************************************************
* Outline 		: TS_HourSecondSensor_Init
* Description 	: IIC hardware Initialization for second sensor.
* Argument  	: none
* Return value  : none
******************************************************************************/
static void TS_HourSecondSensor_Init(void)
{
	  TS_HourSecondSensorInterrupt_Init();

	  PMAPPWD = PMAPKEY;                        // Get write-access to port mapping regs
	  P1MAP3  = PM_NONE;                     	// PM_NONE output to P1.3
	  P1MAP2  = PM_NONE;                     	// PM_NONE output to P1.2
	  P2MAP3  = PM_UCB0SDA;                     // P2.3 // I2C data
	  P2MAP6  = PM_UCB0SCL;                     // P2.6 // I2C clock
	  PMAPPWD = 0;                              // Lock port mapping registers

	  MINUTE_IIC_SEL &= ~(MINUTE_SCL + MINUTE_SDA);
	  HOUR_IIC_SEL   |= HOUR_SDA + HOUR_SCL;    //  P2.6=SCL, P2.3=SDA

	  TS_SensorIIC_Start();
}
/**************************************************************************//**
* TS_SensorsInterrupt_Clear
* @brief         - Clearing the interrupt status for further sensing.
* @param         -
* @return        -
******************************************************************************/
void TS_SensorsInterrupt_Clear(void)
{
//	  __delay_cycles(50);
	  TS_Sensor_IIC_Write(&BufferInterruptClear,CLEAR_COMMAND_BYTE);  // clearing the interrupts.
}
/**************************************************************************//**
* TS_Config_MinuteSensor
* @brief         - Program for configuring the sensor registers for Proximity read.
* @param         -
* @return        -
******************************************************************************/
static void TS_Config_MinuteSensor(void)
{
	  uint8 ucTotalCommand=0;

	  TS_SensorsInterrupt_Clear();

	  /*Sending the commands for the setting of the sensor registers*/
	  for(ucTotalCommand=0;ucTotalCommand < TOTAL_COMMANDS_MINUTE;ucTotalCommand++)
	  {
//		  __delay_cycles(50);
		  TS_Sensor_IIC_Write(ucMinuteSensorStart[ucTotalCommand],TOTAL_BYTES_SEND);
	  }
}
/**************************************************************************//**
* TS_ConfigHourSecondSensor
* @brief         - Program for configuring the sensor registers for Proximity read.
* @param         -
* @return        -
******************************************************************************/
static void TS_ConfigHourSecondSensor(uint8 ucSelectHand)
{
	  uint8 ucTotalCommand=0;

	  TS_SensorsInterrupt_Clear();

	  /*Sending the commands for the setting of the sensor registers*/
	  if(ucSelectHand == HOUR_HAND)
	  {
		  for(ucTotalCommand=0;ucTotalCommand < TOTAL_COMMANDS_HOUR;ucTotalCommand++)
		  {
//			  __delay_cycles(50);
			  TS_Sensor_IIC_Write(ucHourSensorStart[ucTotalCommand],TOTAL_BYTES_SEND);
		  }
	  }
	  if(ucSelectHand == SECOND_HAND)
	  {
		  for(ucTotalCommand=0;ucTotalCommand < TOTAL_COMMANDS_SECOND;ucTotalCommand++)
		  {
//			  __delay_cycles(50);
			  TS_Sensor_IIC_Write(ucSecondStart[ucTotalCommand],TOTAL_BYTES_SEND);
		  }
	  }
}
/**************************************************************************//**
* TS_MinuteSensorStart
* @brief         - Program for starting the first sensor.
* @param         -
* @return        -
******************************************************************************/
void TS_MinuteSensorStart(uint8 ucInterrupt)
{
	TS_MinuteSensor_Init();
	TS_Config_MinuteSensor();

	if(ucInterrupt == INTERRUPT_ENABLE)
	{
		TS_Sensor_IIC_Write(ucProxiStartWithInt,TOTAL_BYTES_SEND);
	}
	else
	{
		TS_Sensor_IIC_Write(ucProxiStartWithOutInt,TOTAL_BYTES_SEND);
	}
}
/**************************************************************************//**
* TS_HourSecondSensor_Start
* @brief         - Program for starting the second sensor.
* @param         -
* @return        -
******************************************************************************/
void TS_HourSecondSensor_Start(uint8 ucSelectHand,uint8 ucInterrupt)
{
   TS_HourSecondSensor_Init();
   TS_ConfigHourSecondSensor(ucSelectHand);

	if(ucInterrupt == INTERRUPT_ENABLE)
	{
		TS_Sensor_IIC_Write(ucProxiStartWithInt,TOTAL_BYTES_SEND);
	}
	else
	{
		TS_Sensor_IIC_Write(ucProxiStartWithOutInt,TOTAL_BYTES_SEND);
	}
}
/**************************************************************************//**
* TS_Sensor_Stop
* @brief         - Function to disable the sensor and clearing the interrupts.
* @param         -
* @return        -
******************************************************************************/
void TS_Sensor_Stop(void)
{
//	  __delay_cycles(50);
	  TS_Sensor_IIC_Write(ucBufferProxiDisabled,2);   // clearing the enable register.
//	  __delay_cycles(50);
	  TS_Sensor_IIC_Write(&BufferInterruptClear,CLEAR_COMMAND_BYTE);   // clearing the interrupts.

	  UCB0CTL1 |= UCSWRST;                      	  // Enable SW reset, disabling the IIC activity
}
/**************************************************************************//**
* TS_ReturnProximityReadData
* @brief         - Program for returning total GPS seconds from GPS Buffer.
* @param         -
* @return        -
******************************************************************************/
uint16 TS_ReturnProximityReadData(void)
{
	uint8 ucProximityBufferRead[2];
	uint16 usProxiReadData;

	TS_Sensor_IIC_Read(&ucProxiRead,ucProximityBufferRead);
	usProxiReadData=((uint16)((uint16)ucProximityBufferRead[1] << SHIFT) \
									  + (uint16)ucProximityBufferRead[0]);
	return usProxiReadData;
}
/**************************************************************************//**
* TS_Sensor_IIC_Write
* @brief         - Function for IIC sensor write.
* @param         - pointer to which the register to be write, total number of bytes to write
* @return        - uint8 if returned 0 IIC is free.
******************************************************************************/
uint8 TS_Sensor_IIC_Write (const unsigned char *ucWriteBuffer, uint8 ucTotalBytes)
{
	  uint8 ucIICstatusByte=0;

	  PTxData   = ucWriteBuffer; 					   // TX array start address
	  TXByteCtr = ucTotalBytes;

	  UCB0IE &= ~(UCRXIE);
	  UCB0IE |= (UCTXIE);
	  UCB0CTL1 |= UCTR + UCTXSTT;             	 		// I2C TX, start condition
	  __bis_SR_register(LPM3_bits);     				// Enter LPM0, enable interrupts
	  __no_operation();

	  while (UCB0CTL1 & UCTXSTP);             			// Ensure stop condition got sent, receiver mode,UCTXSTP is
														//  automatically cleared after STOP is generated.
	  ucIICstatusByte = (UCB0STAT & UCBUSY);

	  return ucIICstatusByte;							// returning the Status register
	  	  	  	  	  	  	  	  	  	  	  	  	  	// If returned 0 IIC is free after operation
}
/**************************************************************************//**
* TS_Sensor_IIC_Read
* @brief         - Function for IIC sensor read.
* @param         - pointer to the register to be read, pointer to which the data is to be read.
* @return        - uint8 if returned 0 IIC is free.
******************************************************************************/
uint8 TS_Sensor_IIC_Read (const unsigned char *ucWriteCommand, uint8 *ucReadBuffer)
{
	  uint8 ucIICstatusByte=0;

	  PTxData = ucWriteCommand; 						// TX array start address
	  PRxData = ucReadBuffer;    						// Start of RX buffer
	  TXByteCtr = 1;									// total number of commands
	  RXByteCtr = sizeof ucReadBuffer;                  // Load RX byte counter

	  UCB0IE |= UCTXIE + UCSTTIE + UCSTPIE;				// enabling the TX
	  UCB0CTL1 |= UCTR + UCTXSTT;             	 		// I2C start and as master
	   __bis_SR_register(LPM3_bits);     				// Enter LPM0, enable interrupts
	   __no_operation();                       			// Remain in LPM0 until all data
	                                            		// is TX'd

	   while (UCB0CTL1 & UCTXSTP);
	   UCB0CTL1 &= ~(UCTR);								// for selecting the receive operation Libin
	   UCB0IE   |= UCRXIE;
	   UCB0CTL1 |= UCTXSTT;                    			// I2C start and receiver
	   __bis_SR_register(LPM3_bits);    				// Enter LPM0, enable interrupts
	   __no_operation();                       			// Remain in LPM0 until all data

	   while (UCB0CTL1 & UCTXSTP);

	   ucIICstatusByte =(UCB0STAT & UCBUSY);
	   return ucIICstatusByte;							// returning the Status register
	  	  	  	  	  									// If returned 0 IIC is free after operation
}
//------------------------------------------------------------------------------
// The USCIAB0TX_ISR is structured such that it can be used to transmit any
// number of bytes by pre-loading TXByteCtr with tz`he byte count. Also, TXData
// points to the next byte to transmit.
//------------------------------------------------------------------------------
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
{
  switch(__even_in_range(UCB0IV,12))
  {
	  case  0: break;                          			// Vector  0: No interrupts
	  case  2: break;                           		// Vector  2: ALIFG
	  case  4: break;                           		// Vector  4: NACKIFG
	  case  6: break;                           		// Vector  6: STTIFG
	  case  8: break;                           		// Vector  8: STPIFG
	  case 10:											// Vector 10: RXIFG
		  	  	RXByteCtr--;                            // Decrement RX byte counter.
				if (RXByteCtr)
				{
				  *PRxData++ = UCB0RXBUF;               // Move RX data to address PRxData
				  if (RXByteCtr == 1)                   // Only one byte left?
				  {
					UCB0CTL1 |= UCTXSTP;                // Generate I2C stop condition
				  }
				}
				else
				{
				  *PRxData = UCB0RXBUF;                 // Move final RX data to PRxData
				  UCB0IFG &= ~UCRXIFG;                  // Clear USCI_B0 RX int flag
				  UCB0IE &= ~(UCRXIE);  				// clearing the RX interrupt

				  __bic_SR_register_on_exit(LPM3_bits);
				}
			   break;
	  case 12:                                   		// Vector 12: TXIFG
				if (TXByteCtr)                          // Check TX byte counter
				{
				  UCB0TXBUF = *PTxData++;               // Load TX buffer
				  TXByteCtr--;                          // Decrement TX byte counter
				}
				else
				{
				  UCB0CTL1 |= UCTXSTP;                  // I2C stop condition
				  UCB0IFG  &= ~UCTXIFG;                 // Clear USCI_B0 TX int flag
				  UCB0IE   &= ~(UCTXIE);  				// clearing the transmit interrupt

				  __bic_SR_register_on_exit(LPM3_bits);
				}
				break;
	  default:  break;
  }
}


