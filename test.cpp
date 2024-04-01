//////////////////////////////////////////////////////////////////////////
///AD7147-1 Driver by Md Shafiqur Rahman & Jacob Girgis

#include <Arduino.h>			// Use the arduino functions (digitalWrite/Read, analogRead, tone, Serial, .etc)
#include <Wire.h>				// used for I2C communication
#include "avr/pgmspace.h" 		// allows for better memory allocation
#include <stdlib.h>				// standard library
#include <inttypes.h> 			// recognize uint8_t, uint16_t and other types
#include <time.h>
#include <stdio.h>

//REGISTERS and ADDRESSES
#define AD7147_ADDR 0x2C  
#define PWR_CONTROL 0x000
#define STAGE_CAL_EN 0x001
#define AMB_COMP_CTRL0 0x002
#define AMB_COMP_CTRL1 0x003
#define AMB_COMP_CTRL2 0x004
#define STAGE_LOW_INT_ENABLE 0x005
#define STAGE_HIGH_INT_ENABLE 0x006
#define STAGE_COMPLETE_INT_ENABLE 0x007
#define STAGE_LOW_INT_STATUS 0x008
#define STAGE_HIGH_INT_STATUS 0x009
#define STAGE_COMPLETE_INT_STATUS 0x00A
#define STAGE0_CONNECTION60 0x080
#define STAGE0_CONNECTION127 0x081
#define STAGE0_AFE_OFFSET 0x082
#define STAGE1_CONNECTION60 0x088
#define STAGE1_CONNECTION127 0x089
#define STAGE1_AFE_OFFSET 0x08A
#define STAGE2_CONNECTION60 0x090
#define STAGE2_CONNECTION127 0x091
#define STAGE2_AFE_OFFSET 0x092
#define STAGE3_CONNECTION60 0x098
#define STAGE3_CONNECTION127 0x099
#define STAGE4_CONNECTION60 0x0A0
#define STAGE4_CONNECTION127 0x0A1
#define STAGE5_CONNECTION60 0x0A8
#define STAGE5_CONNECTION127 0x0A9
#define STAGE6_CONNECTION60 0x0B0
#define STAGE6_CONNECTION127 0x0B1
#define STAGE7_CONNECTION60 0x0B8
#define STAGE7_CONNECTION127 0x0B9
#define STAGE8_CONNECTION60 0x0C0
#define STAGE8_CONNECTION127 0x0C1
#define STAGE9_CONNECTION60 0x0C8
#define STAGE9_CONNECTION127 0x0C9
#define STAGE10_CONNECTION60 0x0D0
#define STAGE10_CONNECTION127 0x0D1
#define STAGE11_CONNECTION60 0x0D8
#define STAGE11_CONNECTION127 0x0D9
#define CDC_RESULT_S0 0x00B
#define CDC_RESULT_S1 0x00C
#define CDC_RESULT_S2 0x00D


// this function reads from an register and returns the 16 bit register value
uint16_t readByte(uint16_t address) {  
  
  uint16_t rx1Byte, rx2Byte, rxFinal16bitbyte;
  
  Wire.beginTransmission(AD7147_ADDR);					// Begin Transmission to the 7 bit i2c address				
  
  Wire.write((address >> 8) & 0xFF); 					// writes the upper byte		

  Wire.write((address) & 0xFF);							// writes the lower byte	
	
  int i2cStatus = Wire.endTransmission(false);		    // i2cStatus will return 1 if there is an error. 
  _delay_ms(1); 										// delay for 1 ms just in case it takes too long.
  
  Wire.requestFrom(AD7147_ADDR, 2); 			   	    //Wire.availible will see how much data is in the buffer, only go into the loop if there is non zero amount of data
  if (Wire.available()>=1) { 
    rx1Byte = Wire.read();								 // get the upper byte (8 bits)
    rx2Byte = Wire.read();							 	 //get the lower byte (8 bits)
    rxFinal16bitbyte = (rx1Byte << 8) | rx2Byte;	     //combined of the two 8 bits to a 16 bit
    return rxFinal16bitbyte; 							// return final number 
  }
  else {
	// return an error code
    return -1;
  }
}

// this function will block write to a specific register
bool writeByte(uint16_t address, uint16_t data16bit) { 				// register address and data value to be sent as an argument
  Wire.beginTransmission(AD7147_ADDR); 								// Begin Transmission to the 7 bit i2c address, so this is auomatically writing the 7 bit address?
  Wire.write((address >> 8) & 0xFF);								 // writes the upper byte		
  Wire.write((address) & 0xFF);										 // writes the lower byte		
  Wire.write((data16bit >> 8) & 0xFF);								// write the data value upper byte
  Wire.write((data16bit) & 0xFF);									// write the data value lower byte
  int i2cStatus = Wire.endTransmission();							// write all the commands in the queue and close the connection
  _delay_ms(1);														// let it do its thing, in case it takes a long time
  if (i2cStatus)													// return false if i2cStatus is 1, indicating an error
    return false;
  else
    return true;
}



void writeStage0_Connection60(){
	
	writeByte(STAGE0_CONNECTION60,0b0011111111111110);		//CIN0 Connected to CDC Positive input, all other CINx are connected to BIAS
}

void writeStage0_Connection127(){
	
	writeByte(STAGE0_CONNECTION127,0b0001111111111111);		
}

void writeStage1_Connection60(){
	
	writeByte(STAGE1_CONNECTION60,0b0011111111111011);		//CIN1 Connected to CDC Positive input, all other CINx are connected to BIAS
}

void writeStage1_Connection127(){
	
	writeByte(STAGE1_CONNECTION127,0b0001111111111111);
}

void writeStage2_Connection60(){
	
	writeByte(STAGE2_CONNECTION60,0b0011111111101111);		//CIN2 Connected to CDC Positive input, all other CINx are connected to BIAS
}

void writeStage2_Connection127(){
	
	writeByte(STAGE2_CONNECTION127,0b0001111111111111);
}

void writeStage3_Connection60(){
	
	writeByte(STAGE3_CONNECTION60,0b0011111111111111);		//all CINx are connected to BIAS
}

void writeStage3_Connection127(){
	
	writeByte(STAGE3_CONNECTION127,0b1100111111111111);		
}

void writeStage4_Connection60(){
	
	writeByte(STAGE4_CONNECTION60,0b0011111111111111);		//all CINx are connected to BIAS
}

void writeStage4_Connection127(){
	
	writeByte(STAGE4_CONNECTION127,0b1100111111111111);
}

void writeStage5_Connection60(){
	
	writeByte(STAGE5_CONNECTION60,0b0011111111111111);		//all CINx are connected to BIAS
}

void writeStage5_Connection127(){
	
	writeByte(STAGE5_CONNECTION127,0b1100111111111111);
}

void writeStage6_Connection60(){
	
	writeByte(STAGE6_CONNECTION60,0b0011111111111111);		//all CINx are connected to BIAS
}

void writeStage6_Connection127(){
	
	writeByte(STAGE6_CONNECTION127,0b1100111111111111);
}

void writeStage7_Connection60(){
	
	writeByte(STAGE7_CONNECTION60,0b0011111111111111);		//all CINx are connected to BIAS
}

void writeStage7_Connection127(){
	
	writeByte(STAGE7_CONNECTION127,0b1100111111111111);
}

void writeStage8_Connection60(){
	
	writeByte(STAGE8_CONNECTION60,0b0011111111111111);		//all CINx are connected to BIAS
}

void writeStage8_Connection127(){
	
	writeByte(STAGE8_CONNECTION127,0b1100111111111111);
}

void writeStage9_Connection60(){
	
	writeByte(STAGE9_CONNECTION60,0b0011111111111111);		//all CINx are connected to BIAS
}

void writeStage9_Connection127(){
	
	writeByte(STAGE9_CONNECTION127,0b1100111111111111);
}

void writeStage10_Connection60(){
	
	writeByte(STAGE10_CONNECTION60,0b0011111111111111);		//all CINx are connected to BIAS
}

void writeStage10_Connection127(){
	
	writeByte(STAGE10_CONNECTION127,0b1100111111111111);
}

void writeStage11_Connection60(){
	
	writeByte(STAGE11_CONNECTION60,0b0011111111111111);		//all CINx are connected to BIAS
}

void writeStage11_Connection127(){
	
	writeByte(STAGE11_CONNECTION127,0b1100111111111111);
}

void writeStage0_Afe_Offset(){
	
	writeByte(STAGE0_AFE_OFFSET,0b1000010010010111);
	
}

void writeStage1_Afe_Offset(){
	
	writeByte(STAGE1_AFE_OFFSET,0b1000001010010001);
	
}

void writeStage2_Afe_Offset(){
	
	writeByte(STAGE2_AFE_OFFSET,0b1000001110010100);
	
}

void writePwr_Control(){
  	  writeByte(PWR_CONTROL,0b0000101000100000);		

}

void writeStage_Cal_En(){
	
	writeByte(STAGE_CAL_EN,0b0000000000000111);				// Calibration for STAGE 0,1,2 are enabled 
}

void writeStage_Low_Int_Enable(){
	
	writeByte(STAGE_LOW_INT_ENABLE,0b0000000000000000);		// Low interrupt disabled for all the stages
}

void writeStage_Hight_Int_Enable(){
	
	writeByte(STAGE_HIGH_INT_ENABLE,0b0000000000000000);	// High interrupt disabled for all the stages 
	
}

void writeStage_Complete_Int_Enable(){
	
	writeByte(STAGE_COMPLETE_INT_ENABLE,0b0000000000000100);	// Conversion complete interrupt after STAGE 2 
}

void readStage_Complete_Int_Status(){
	
	readByte(STAGE_COMPLETE_INT_STATUS);
}

int main(){ // this function runs immeadiately upon upload
  
  init(); 							// calls some arduino intializing to allow the arduino library to be used
  Serial.begin(9600);				// Start USART0 (TX0 and RX0)
  Wire.begin();	    // Start the Wire library
  

  
  writeStage0_Connection60();
  writeStage0_Connection127();
  writeStage0_Afe_Offset();
  writeStage1_Connection60();
  writeStage1_Connection127();
  writeStage1_Afe_Offset();
  writeStage2_Connection60();
  writeStage2_Connection127();
  writeStage2_Afe_Offset();
  writeStage3_Connection60();
  writeStage3_Connection127();
  writeStage4_Connection60();
  writeStage4_Connection127();
  writeStage5_Connection60();
  writeStage5_Connection127();
  writeStage6_Connection60();
  writeStage6_Connection127();
  writeStage7_Connection60();
  writeStage7_Connection127();
  writeStage8_Connection60();
  writeStage8_Connection127();
  writeStage9_Connection60();
  writeStage9_Connection127();
  writeStage10_Connection60();
  writeStage10_Connection127();
  writeStage11_Connection60();
  writeStage11_Connection127();
 
  writePwr_Control();
  
  readStage_Complete_Int_Status();


  writeStage_Low_Int_Enable();
  writeStage_Hight_Int_Enable();
  writeStage_Complete_Int_Enable();
  
  writeStage_Cal_En();
  
  readStage_Complete_Int_Status();
  
  // print what is in the power control register this will be in decimal form, so convert it later
  Serial.print("PWR_CONTROL""\t");
  Serial.println(readByte(PWR_CONTROL)); 
  
  
  
  Serial.print("\nSTAGE_CAL_EN""\t");
  Serial.println(readByte(STAGE_CAL_EN));
  
  
  
  Serial.print("\nStage0_Connection[6:0]""\t");
  Serial.println(readByte(STAGE0_CONNECTION60));
  
  Serial.print("\nStage0_Connection[12:7]""\t");
  Serial.println(readByte(STAGE0_CONNECTION127));
  
  Serial.print("\nStage1_Connection[6:0]""\t");
  Serial.println(readByte(STAGE1_CONNECTION60));
  
  Serial.print("\nStage1_Connection[12:7]""\t");
  Serial.println(readByte(STAGE1_CONNECTION127));
  
  Serial.print("\nStage2_Connection[6:0]""\t");
  Serial.println(readByte(STAGE2_CONNECTION60));
  
  Serial.print("\nStage2_Connection[12:7]""\t");
  Serial.println(readByte(STAGE2_CONNECTION127));
  

  
  Serial.println(readByte(STAGE0_AFE_OFFSET));
  Serial.println(readByte(STAGE1_AFE_OFFSET));
  Serial.println(readByte(STAGE2_AFE_OFFSET));
   
while (1) {  
	  
	 
	 Serial.print(readByte(CDC_RESULT_S0));
	 Serial.print("\t");
	 Serial.print(readByte(CDC_RESULT_S1));
	 Serial.print("\t");
	 Serial.print(readByte(CDC_RESULT_S2));
	 Serial.print("\n");
	
  }
  return(0);
}

