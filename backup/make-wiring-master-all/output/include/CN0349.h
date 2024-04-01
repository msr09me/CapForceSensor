/*
  CN0349.h
*/
#ifndef CN0349_h
#define CN0349_h

#include "Arduino.h"
#include <Wire.h>// used for I2C communication
#include "avr/pgmspace.h"
#define AD5934_ADDR 0x0D   //i2c addresses of CN-0349
#define ADG715_ADDR 0x48

///////////////////////////////////////////////////////////////////////////////////////////
//Constants
///////////////////////////////////////////////////////////////////////////////////////////
//cn-0349
const float CLOCK_SPEED = 16.776 * pow(10, 6); // AD5934 has internal clock of 16.776 MHz
const int validImpedanceData = 2;   //when the data is good
const int validSweep = 4;   // when the sweep is over
//const float alpha = 0.021; // 2.1%°C is the temperature coefficient at 25°C for sea water
const int GF_low_addr = 0;
const int NOS_low_addr = 8;
const int GF_high_addr = 16;
const int NOS_high_addr = 24;
const int slope_addr = 50;
const int intercept_addr = 64;
const float R2 = 100;
const float R3 = 100;
const float R4 = 1000;
const float R7 = 10000;
const float R8 = 1000;
const float R9 = 100;

///////////////////////////////////////////////////////////////////////////////////////////
//Register Map of CN-0349
///////////////////////////////////////////////////////////////////////////////////////////
const int CONTROL_REGISTER[2] =                    { 0x80, 0x81       }; // see mapping below
const int START_FREQUENCY_REGISTER[3] =            { 0x82, 0x83, 0x84 }; // 24 bits for start frequency
const int FREQ_INCREMENT_REGISTER[3] =             { 0x85, 0x86, 0x87 }; // 24 bits for frequency increment
const int NUM_INCREMENTS_REGISTER[2] =             { 0x88, 0x89       }; //  9 bits for # of increments
const int NUM_SETTLING_CYCLES_REGISTER[2] =        { 0x8A, 0x8B       }; //  9 bits + 1 modifier for # of settling times
const int STATUS_REGISTER[1] =                     { 0x8F             }; // see mapping below
const int REAL_DATA_REGISTER[2] =                  { 0x94, 0x95       }; // 16-bit, twos complement format
const int IMAG_DATA_REGISTER[2] =                  { 0x96, 0x97       }; // 16-bit, twos complement format

///////////////////////////////////////////////////////////////////////////////////////////
//Control register map @ 0x80, 0x81 of CN-0349
///////////////////////////////////////////////////////////////////////////////////////////
// Note:
//0x80 contains D15 to D8
//0x81 contains D7 to D0

// Control register map (D15 to D12)
const int DEFAULT_VALUE =  B0000; // initial setting
const int INITIALIZE    =  B0001; // excite the unknown impedance initially
const int START_SWEEP   =  B0010; // begin the frequency sweep
const int INCREMENT     =  B0011; // step to the next frequency point
const int REPEAT_FREQ   =  B0100; // repeat the current frequency point measurement
const int POWER_DOWN    =  B1010; // VOUT and VIN are connected internally to ground
const int STANDBY       =  B1011; // VOUT and VIN are connected internally to ground

// D11 = no operation

// Control register map (D10 to D9)
// D8 = PGA gain (0 = x5, 1 = x1) // amplifies the response signal into the ADC
// D7 = reserved, set to 0
// D6 = reserved, set to 0
// D5 = reserved, set to 0
// D4 = reset // interrupts a frequency sweep
// D3 = external system clock, set to 1; internal system clock, set to 0
// D2 = reserved, set to 0
// D1 = reserved, set to 0
// D0 = reserved, set to 0

///////////////////////////////////////////////////////////////////////////////////////////
//Start Frequency Register Code @ 0x82, 0x83, 0x84 of CN-0349
///////////////////////////////////////////////////////////////////////////////////////////
// Start Frequency Code = (2^27)*(startFreqHz)/(CLOCK_SPEED/16)
// however it needs to be split into three values because it is has three registers

///////////////////////////////////////////////////////////////////////////////////////////
// number of increments register @ 0x88, 0x89 of CN-0349
///////////////////////////////////////////////////////////////////////////////////////////
// 0x88: D15 to D9 -- don't care
// 0x89: D8 -- number of increments bit 1
// 0x89: D7 to D0 -- number of increments bits 2 through 9; 9-bit integer number stored in binary format

///////////////////////////////////////////////////////////////////////////////////////////
// number of settling times @ 0x8A, 0x8B of CN-0349
///////////////////////////////////////////////////////////////////////////////////////////
// 0x8A: D15 to D11 -- don't care
// 0x8A: D10 to D9 -- 2-bit decode
//        0 0 = default
//        0 1 = # of cycles x 2
//        1 0 = reserved
//        1 1 = # of cycles x 4
// 0x8A: D8 -- MSB number of settling times
// 0x8B: D7 to D0 -- number of settling times; 9-bit integer number stored in binary format

///////////////////////////////////////////////////////////////////////////////////////////
//Status Register @ 0x8F of CN-0349
///////////////////////////////////////////////////////////////////////////////////////////
//0000 0001 Reserved
//0000 0010 Valid real/imaginary data
//0000 0100 Frequency sweep complete
//0000 1000 Reserved
//0001 0000 Reserved
//0010 0000 Reserved
//0100 0000 Reserved
//1000 0000 Reserved

///////////////////////////////////////////////////////////////////////////////////////////
//ADG715 switches of CN-0349
///////////////////////////////////////////////////////////////////////////////////////////
//calibrating
//      Rcal(ohms) | Rfb(ohms)  |Channels
//RTD:   R3(100)     R9(100)      4,1
//High1: R3(100)     R9(100)      4,1
//High2: R4(1000)    R9(100)      5,1
//Low1:  R4(1000)    R8(1000)     5,2
//Low2:  R7(10000)   R8(1000)     6,2

//measuring
//      Rfb(ohms)  |Channels
//RTD:   R3(100)       1,7
//High:  R3(100)       1,8
//Low:   R4(1000)      2,8

class CN0349{
	public:
		void condSetup();
		void configureAD5934(int settlingTimes, float startFreq, float freqIncr, int numIncr);
		float calibrate(double rcal, double rfb);
		int measure(float GF_rtd, float GF, double NOS, float slope, float intercept, char state, float* Y_cell, float* T_cell, float* YT_cell);  //high or low measurment ranges
 	private:
	    int checkStatus();
		boolean AD5934byteWrite(int address, int data);
		int AD5934byteRead(int address);
		byte frequencyCode(float freqInHz, int byteNum);
		boolean setStartFrequency(float freqInHz);
		boolean setFrequencyIncrement(float freqInHz);
		boolean setNumberOfIncrements(int n);
		boolean setNumberOfSettlingTimes(int n);
		boolean setControlRegister(int code);
		boolean setControlRegister2(); //initalize D11 D10 D9 D8 @0x80 Excitation Voltage 2.0Vp-p, Internal PGA=1
		float sweep(int switch1, int switch2); //performs frequency sweep for real and unreal components, returns the magnitude
		float tempcondtosal(float cond, float temp); //convert microsiemens to salinity, valid for 2 to 42 ppt
		byte ADG715CH(int channel);   //checks channel numbers
		byte ADG715read(byte channel);  //if channel exceeds 9, read all
		void ADG715writeChannel(byte channel, byte state); //change status of a specified channel (1-8)
		void ADG715reset(); //clear out register
		};
#endif
