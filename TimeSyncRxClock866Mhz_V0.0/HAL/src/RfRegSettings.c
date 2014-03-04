
#include "RF1A.h"

#define MHZ_866					// Select the center frequency

#ifdef MHZ_915

// Chipcon
// Product = CC430Fx13x
// Chip version = C   (PG 0.7)
// Crystal accuracy = 10 ppm
// X-tal frequency = 26 MHz
// RF output power = 0 dBm
// RX filterbandwidth = 101.562500 kHz
// Deviation = 19 kHz
// Datarate = 38.383484 kBaud
// Modulation = (1) GFSK
// Manchester enable = (0) Manchester disabled
// RF Frequency = 914.999969 MHz
// Channel spacing = 199.951172 kHz
// Channel number = 0
// Optimization = -
// Sync mode = (3) 30/32 sync word bits detected
// Format of RX/TX data = (0) Normal mode, use FIFOs for RX and TX
// CRC operation = (1) CRC calculation in TX and CRC check in RX enabled
// Forward Error Correction = 
// Length configuration = (0) Fixed packet length, packet length configured by PKTLEN
// Packetlength = 61
// Preamble count = (2)  4 bytes
// Append status = 1
// Address check = (0) No address check
// FIFO autoflush = 0
// Device address = 0
// GDO0 signal selection = ( 6) Asserts when sync word has been sent / received, and de-asserts at the end of the packet
// GDO2 signal selection = (41) RF_RDY
RF_SETTINGS rfSettings = {
    0x08,   // FSCTRL1   Frequency synthesizer control.
    0x00,   // FSCTRL0   Frequency synthesizer control.
    0x23,   // FREQ2     Frequency control word, high byte.
    0x31,   // FREQ1     Frequency control word, middle byte.
    0x3B,   // FREQ0     Frequency control word, low byte.
    0xCA,   // MDMCFG4   Modem configuration.
    0x83,   // MDMCFG3   Modem configuration.
    0x93,   // MDMCFG2   Modem configuration.
    0x22,   // MDMCFG1   Modem configuration.
    0xF8,   // MDMCFG0   Modem configuration.
    0x00,   // CHANNR    Channel number.
    0x34,   // DEVIATN   Modem deviation setting (when FSK modulation is enabled).
    0x56,   // FREND1    Front end RX configuration.
    0x10,   // FREND0    Front end TX configuration.
    0x18,   // MCSM0     Main Radio Control State Machine configuration.
    0x16,   // FOCCFG    Frequency Offset Compensation Configuration.
    0x6C,   // BSCFG     Bit synchronization Configuration.
    0x43,   // AGCCTRL2  AGC control.
    0x40,   // AGCCTRL1  AGC control.
    0x91,   // AGCCTRL0  AGC control.
    0xE9,   // FSCAL3    Frequency synthesizer calibration.
    0x2A,   // FSCAL2    Frequency synthesizer calibration.
    0x00,   // FSCAL1    Frequency synthesizer calibration.
    0x1F,   // FSCAL0    Frequency synthesizer calibration.
    0x59,   // FSTEST    Frequency synthesizer calibration.
    0x81,   // TEST2     Various test settings.
    0x35,   // TEST1     Various test settings.
    0x09,   // TEST0     Various test settings.
    0x47,   // FIFOTHR   RXFIFO and TXFIFO thresholds.
    0x29,   // IOCFG2    GDO2 output pin configuration.
    0x06,   // IOCFG0    GDO0 output pin configuration. Refer to SmartRF® Studio User Manual for detailed pseudo register explanation.
    0x04,   // PKTCTRL1  Packet automation control.
    0x04,   // PKTCTRL0  Packet automation control.
    0x00,   // ADDR      Device address.
    0x64    // PKTLEN    Packet length.
    0xB4,    // SYNC1    Packet length.		// changed password libin
    0x69     // SYNC0    Packet length.		// changed password
};

#elif defined MHZ_868

// Chipcon
// Product = CC430Fx13x
// Crystal accuracy = 10 ppm
// X-tal frequency = 26 MHz
// RF output power = 0 dBm
// RX filter bandwidth = 58.035714 kHz
// Deviation = 3.967285 kHz
// Data rate = 1.19948 kBaud
// Modulation = (1) GFSK
// Manchester enable = (0) Manchester disabled
// RF Frequency = 867.999939 MHz
// Channel spacing = 207.885742 kHz
// Channel number = 0
// Optimization = -
// Sync mode = 16/16 sync word bits detected (0x69, 0xB4)
// Format of RX/TX data = (0) Normal mode, use FIFOs for RX and TX
// CRC operation = (1) CRC calculation in TX and CRC check in RX enabled
// Forward Error Correction = 
// Length configuration = (0) Fixed packet length, packet length configured by PKTLEN
// Packetlength = 62
// Preamble count = (2)  4 bytes
// Append status = 1
// Address check = (0) No address check
// FIFO autoflush = 0
// Device address = 0
// GDO0 signal selection = ( 6) Asserts when sync word has been sent / received, and de-asserts at the end of the packet
// GDO2 signal selection = (41) RF_RDY
RF_SETTINGS rfSettings = {
    0x06,   // FSCTRL1   Frequency synthesizer control. The desired IF frequency to employ in RX
    0x00,   // FSCTRL0   Frequency synthesizer control. Frequency offset added to the base frequency before being used by the
    	    // frequency synthesizer (2s complement).

    0x21,   // FREQ2     Frequency control word, high byte. 0x20,  20 (867.999985)  0x21(867.999939)ori, changed from  to
    0x62,   // FREQ1     Frequency control word, middle byte. 0x34,25               0x62,
    0x76,   // FREQ0     Frequency control word, low byte. 0x62,   ED               0x76,

    /*Determine the data rate of the reception of transmission*/
    0xF5,   // MDMCFG4   channel bandwidth and symbol rate	// 60Khz
    0x83,   // MDMCFG3   symbol rate
    0x12,   // MDMCFG2  /*GFSK 0x93, // 16/16 word detection, better sensitivity enable, manchester disable*/
    0x23,   // MDMCFG1   preamble bytes, channel spacing 0x22 ori, 23
    0x06,   // MDMCFG0   channel spacing  0xF8 ori, 06,

    0x00,   // CHANNR    Channel number.
    0x12,   // DEVIATN   Modem deviation setting 4.9Khz(when FSK modulation is enabled). //0x34, //0x11(4Khz) ori, changed 12

    0x56,   // FREND1    Front end RX configuration.
    0x10,   // FREND0    Selects PA power setting. Adjusts current TX LO buffer

    0x18,   // MCSM0     Automatically calibrate When going from IDLE to RX or TX (or FSTXON) .
    		// 0x18 changed for checking the current

    0x16,   // FOCCFG    The frequency compensation loop gain(3K),The frequency compensation loop gain to be used after a sync word is
    	    // detected(K/2),The saturation point for the frequency offset compensation algorithm

    0x6C,   // BSCFG     The clock recovery feedback loop integral gain.The clock recovery feedback loop proportional gain
    		// The clock recovery feedback loop integral gain to be used after a sync word is detected.

    0x03,   // AGCCTRL2  AGC control.
    0x40,   // AGCCTRL1  AGC control.
    0x91,   // AGCCTRL0  AGC control.

    0xE9,   // FSCAL3    Frequency synthesizer calibration.
    0x2A,   // FSCAL2    Frequency synthesizer calibration.
    0x00,   // FSCAL1    Frequency synthesizer calibration.
    0x1F,   // FSCAL0    Frequency synthesizer calibration.

    0x59,   // FSTEST    Frequency synthesizer calibration.For test only
    0x81,   // TEST2     Various test settings.
    0x2D,   // TEST1     Various test settings.
    0x09,   // TEST0     Various test settings.

    0x4F,   // FIFOTHR   RXFIFO and TXFIFO thresholds.RX Attenuation 0,60 bytes in the RX FIFO

    0x29,   // IOCFG2    GDO2 output pin configuration.
    0x07,   // IOCFG0    GDO0 output pin configuration./* 0x06, asserts when sync word received*/

    0x00,   // PKTCTRL1  No address check,append status bytes disabled. changed libin  0x04(status bytes enable),
    0x05,   // PKTCTRL0  Whitening off.Format of RX and TX data CRC enable ,Variable packet length mode
    		// 0x04 changed libin

    0x00,    // ADDR     Device address.
    0xFF,    // PKTLEN   Packet length.  0xFF ori, 0x1E

    0xB4,    // SYNC1    Packet length.		// changed password libin
    0x69     // SYNC0    Packet length.		// changed password
};

#elif defined MHZ_866

// Chipcon
// Product = CC430Fx13x
// Chip version = C   (PG 0.7)
// Crystal accuracy = 10 ppm
// X-tal frequency = 26 MHz
// RF output power = 0 dBm
// RX filter bandwidth = 58.035714 kHz
// Deviation = 3.967285 kHz
// Data rate = 1.19948 kBaud
// Modulation = (1) GFSK
// Manchester enable = (0) Manchester disabled
// RF Frequency = 866 MHz
// Channel spacing = 207.885742 kHz
// Channel number = 0
// Optimization = -
// Sync mode = 16/16 sync word bits detected (0x69, 0xB4)
// Format of RX/TX data = (0) Normal mode, use FIFOs for RX and TX
// CRC operation = (1) CRC calculation in TX and CRC check in RX enabled
// Forward Error Correction =
// Length configuration = (0) Fixed packet length, packet length configured by PKTLEN
// Packetlength = 62
// Preamble count = (2)  4 bytes
// Append status = 1
// Address check = (0) No address check
// FIFO autoflush = 0
// Device address = 0
// GDO0 signal selection = ( 6) Asserts when sync word has been sent / received, and de-asserts at the end of the packet
// GDO2 signal selection = (41) RF_RDY
RF_SETTINGS rfSettings = {
    0x06,   // FSCTRL1
    0x00,   // FSCTRL0

    0x21,   // FREQ2     Frequency control word, high byte.   carrier signal as 866Mhz
    0x4E,   // FREQ1     Frequency control word, middle byte.
    0xC4,   // FREQ0     Frequency control word, low byte.

    0xF5,   // MDMCFG4
    0x83,   // MDMCFG3
    0x12,   // MDMCFG2
    0x23,   // MDMCFG1
    0x06,   // MDMCFG0
    0x00,   // CHANNR
    0x12,   // DEVIATN
    0x56,   // FREND1
    0x10,   // FREND0
    0x18,   // MCSM0
    0x16,   // FOCCFG
    0x6C,   // BSCFG
    0x03,   // AGCCTRL2  AGC control.
    0x40,   // AGCCTRL1  AGC control.
    0x91,   // AGCCTRL0  AGC control.
    0xE9,   // FSCAL3    Frequency synthesizer calibration.
    0x2A,   // FSCAL2    Frequency synthesizer calibration.
    0x00,   // FSCAL1    Frequency synthesizer calibration.
    0x1F,   // FSCAL0    Frequency synthesizer calibration.
    0x59,   // FSTEST    Frequency synthesizer calibration.For test only
    0x81,   // TEST2     Various test settings.
    0x2D,   // TEST1     Various test settings.
    0x09,   // TEST0     Various test settings.
    0x4F,   // FIFOTHR
    0x29,   // IOCFG2    GDO2
    0x07,   // IOCFG0    GDO0
    0x00,   // PKTCTRL1
    0x05,   // PKTCTRL0
    0x00,   // ADDR
    0xFF,   // PKTLEN
    0xB4,   // SYNC1    sync word1.		// changed password libin
    0x69    // SYNC0    sync word.0		// changed password
};
#endif

#if !defined (MHZ_868) && !defined (MHZ_915) && !defined (MHZ_866)
#error "Please select MHZ_868, MHZ_915 or MHZ_866 as the active project configuration"
#endif
