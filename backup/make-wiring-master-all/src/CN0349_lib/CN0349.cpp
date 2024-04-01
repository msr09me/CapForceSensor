/*
  CN0349.cpp
*/

#include "Arduino.h"
#include <Wire.h>// used for I2C communication
#include "avr/pgmspace.h"
#include "CN0349.h"


void CN0349::condSetup() {
  Wire.begin();
  configureAD5934(15, 8 * pow(10, 3), 4, 2);    // number of settling times ,start frequency (Hz),frequency increment (Hz), number of increments
  delay(1000);
}
///////////////////////////////////////////////////////////////////////////////////////////
/////////AD5934
///////////////////////////////////////////////////////////////////////////////////////////
boolean CN0349::AD5934byteWrite(int address, int data) {
  Wire.beginTransmission(AD5934_ADDR);
  Wire.write(address); // address specifier
  Wire.write(data); // value specifier
  int i2cStatus = Wire.endTransmission();
  delay(1);
  if (i2cStatus)
    return false;
  else
    return true;
}

int CN0349::AD5934byteRead(int address) {
  int rxByte;
  Wire.beginTransmission(AD5934_ADDR);
  Wire.write(address); // address specifier
  int i2cStatus = Wire.endTransmission();
  delay(1);
  Wire.requestFrom(AD5934_ADDR, 1);
  if (1 <= Wire.available()) {
    rxByte = Wire.read();
  }
  else {
    rxByte = -1;
  }
  return rxByte;
}

int CN0349::checkStatus() {
  return (AD5934byteRead(STATUS_REGISTER[0]) & 7);
}

// start frequency and frequency increment formula:
byte CN0349::frequencyCode(float freqInHz, int byteNum) {
  long value = long((freqInHz / (CLOCK_SPEED / 16)) * pow(2, 27));
  byte code[3];
  code[0] = (value & 0xFF0000) >> 0x10;
  code[1] = (value & 0x00FF00) >> 0x08;
  code[2] = (value & 0x0000FF);
  return code[byteNum];
}

boolean CN0349::setStartFrequency(float freqInHz) {
  boolean statusValue;
  for (int n = 0; n < 3; n++) {
    statusValue = AD5934byteWrite(START_FREQUENCY_REGISTER[n], frequencyCode(freqInHz, n));
  }
  return statusValue;
}

boolean CN0349::setFrequencyIncrement(float freqInHz) {
  boolean statusValue;
  for (int n = 0; n < 3; n++) {
    statusValue = AD5934byteWrite(FREQ_INCREMENT_REGISTER[n], frequencyCode(freqInHz, n));
  }
  return statusValue;
}

boolean CN0349::setNumberOfIncrements(int n) {
  boolean i2cStatus;
  int numIncrements = min(n, 511);
  i2cStatus = AD5934byteWrite(NUM_INCREMENTS_REGISTER[0], numIncrements >> 8);
  i2cStatus = AD5934byteWrite(NUM_INCREMENTS_REGISTER[1], numIncrements & 255);
  return i2cStatus;
}

boolean CN0349::setNumberOfSettlingTimes(int n) {
  int decode;
  int numSettlingTimes = min(n, 2047);
  if (n > 1023) {
    decode = 3;
    numSettlingTimes /= 4;
  }
  else if (n > 511) {
    decode = 1;
    numSettlingTimes /= 2;
  }
  else {
    decode = 0;
    numSettlingTimes = n;
  }
  boolean i2cStatus;
  i2cStatus = AD5934byteWrite(NUM_SETTLING_CYCLES_REGISTER[0], (numSettlingTimes >> 8) + (decode << 1));
  i2cStatus = AD5934byteWrite(NUM_SETTLING_CYCLES_REGISTER[0], numSettlingTimes & 255);
  return i2cStatus;
}

boolean CN0349::setControlRegister(int code) {
  int rxByte = AD5934byteRead(CONTROL_REGISTER[0]);
  rxByte &= 0x0F; // clear upper four bits
  rxByte |= code << 4; // set to 1011
  boolean s = AD5934byteWrite(CONTROL_REGISTER[0], rxByte);
  delay(1);
  return s;
}

boolean CN0349::setControlRegister2() { //initalize D11 D10 D9 D8 @0x80 Excitation Voltage 2.0Vp-p, Internal PGA=1
  int rxByte = AD5934byteRead(CONTROL_REGISTER[0]);
  rxByte &= 0xF0; // clear lower four bits (11110000)
  rxByte |= B00000001; // set to 0001 Excitation Voltage 2.0Vp-p, Internal PGA=1
  boolean s = AD5934byteWrite(CONTROL_REGISTER[0], rxByte);
  delay(10);
  return s;
}

void CN0349::configureAD5934(int settlingTimes, float startFreq, float freqIncr, int numIncr) {
  setNumberOfSettlingTimes(settlingTimes);
  setStartFrequency(startFreq);
  setFrequencyIncrement(freqIncr);
  setNumberOfIncrements(numIncr);
}


float CN0349::sweep(int switch1, int switch2) { //performs frequency sweep for real and unreal components, returns the magnitude
  float magnitude;
  //configureAD5934(15, 8 * pow(10, 3), 4, 2);    // number of settling times ,start frequency (Hz),frequency increment (Hz), number of increments
  //configureAD5934(2, 1 * pow(10, 5),4,0 );
  //configureAD5934(2, 1 * pow(10, 3), 1 * pow(10, 4),0 );
  //configureAD5934(2, 1 * pow(10, 3), 1 * pow(10, 3),100 );
  //delay(10);
  setControlRegister2();
 //delay(10);
  ADG715reset();            //clear out switches
  //delay(10);
  ADG715writeChannel(switch1, 1); //turn on switchs
  //delay(10);
  ADG715writeChannel(switch2, 1);
  //delay(10);
  int real = 0;
  int imag = 0;
  ////////////////////////////////////////////////0.Inizialize bit D11,D10,D9,D8
  setControlRegister2();
  ////////////////////////////////////////////////1. place AD5934 in standby mode
  setControlRegister(STANDBY);
  ////////////////////////////////////////////////2. initialize with start frequency
  setControlRegister(INITIALIZE);
  delay(100);
  ////////////////////////////////////////////////3. start frequency sweep
  setControlRegister(START_SWEEP);
  //delay(100);
  ////////////////////////////////////////////////4. poll status register until complete
  while (checkStatus() < 6) {
  while (checkStatus() < 4) {
  // for (int i = 0; i < 10; i++) {
    //Serial.print(checkStatus());
    delay(80);
    if (checkStatus() == validImpedanceData) {
      // 5. read values
      real = AD5934byteRead(REAL_DATA_REGISTER[0]) << 8;
      real |= AD5934byteRead(REAL_DATA_REGISTER[1]);

      if (real > 0x7FFF) { // negative value
        real &= 0x7FFF;
        real -= 0x10000;
      }
      imag = AD5934byteRead(IMAG_DATA_REGISTER[0]) << 8;
      imag |= AD5934byteRead(IMAG_DATA_REGISTER[1]);
      if (imag > 0x7FFF) { // negative value
        imag &= 0x7FFF;
        imag -= 0x10000;
      }
      magnitude = sqrt(pow(double(real), 2) + pow(double(imag), 2));
	  double phase = atan(double(imag) / double(real));

	 // Serial.println(magnitude,20);
      setControlRegister(INCREMENT);
	//setControlRegister(REPEAT_FREQ);
    }
  }
  delay(80);
  }
  setControlRegister(POWER_DOWN);
  return magnitude;
}

float CN0349::calibrate(double rcal, double rfb) {
  float magnitude;
  int switch1, switch2 = 0;

  //reference:
  //      Rcal(ohms) | Rfb(ohms)  |Channelscalibrate
  //RTD:   R3(100)     R9(100)      4,1
  //High1: R3(100)     R9(100)      4,1
  //High2: R4(1000)    R9(100)      5,1
  //Low1:  R4(1000)    R8(1000)     5,2
  //Low2:  R7(10000)   R8(1000)     6,2

  if (rcal == 100 && rfb == 100) { //rtd and 1st high calibration, r3, r9
    switch1 = 4;
    switch2 = 1;
  }
  else if (rcal == 1000 && rfb == 100) { //2nd high calibration r4, r9
    switch1 = 5;
    switch2 = 1;
  }
  else if (rcal == 1000 && rfb == 1000) { //1st low calibration r4, r8
    switch1 = 5;
    switch2 = 2;
  }
  else if (rcal == 10000 && rfb == 1000) { //1st low calibration r7, r8
    switch1 = 6;
    switch2 = 2;
  }
  else {
    rfb = 0;
    rcal = 0;
  }
  if (!(rfb && rcal == 0)) {
    magnitude = sweep(switch1, switch2);
  }
  else {
    magnitude = 0;
  }
  return magnitude;
};

int CN0349::measure(float GF_rtd, float GF, double NOS, float slope, float intercept, char state, float* Y_cell, float* T_cell, float* YT_cell) {  //high or low measurment ranges
  const float A = 3.9083 * pow(10, -3);
  const float B = -5.775 * pow(10, -7);
  float imp, T_imp;
  float magnitude = 0;
  int switch1, switch2 = 0;
  int flag = 1;
  float NX, YX = 0;
  //reference
  //      Rfb(ohms)  |Channels
  //RTD:   R9(100)       1,7
  //High:  R9(100)       1,8
  //Low:   R8(1000)      2,8

  //rtd measurement rfb = r9 also path to thermistor
  switch1 = 1;  //measure rtd first
  switch2 = 7;
  magnitude = sweep(switch1, switch2);    //measure temperature
  T_imp = 1 / (magnitude * GF_rtd);
  //Serial.print("Timp:      \t   ");
  //Serial.println(T_imp);
  //*T_cell = 2.4941 * c.T_imp - 249.03; //convert impedence to temperature
    *T_cell = (-A + sqrt(pow(A, 2) - 4 * B * (1 - T_imp / 100))) / (2 * B); //convert impedence to temperature (known pt100 formula)
  //Rt = R0 * (1 + A* t + B*t2 + C*(t-100)* t3)

  if (state == 'H') { //high measurement rfb = r9
    switch1 = 1;
    switch2 = 8;
  }
  else if (state == 'L') { //low measurement rfb=r8
    switch1 = 2;
    switch2 = 8;
  }
  else {
    flag = 0;
	return 0;
  }
  if (!(flag = 0)) {
    magnitude = sweep(switch1, switch2);      //get conductivity magnitude
    //Serial.println(magnitude, 30);
  }
  else {
    magnitude = 0;
  }
  // three point calibration equation
  //YX = (NX-NOS)*GF
  //YCELL = YX / (1 - 100 * YX);
  imp = 1 / (magnitude * GF);
 // Serial.print("imp:      \t   ");
 // Serial.println(imp);
  NX = magnitude;
  YX = (NX - NOS) * GF;
  if (state == 'H') { //high measurement rfb = r9
      *Y_cell = YX / (1 - R2 * YX) * 1000; //1/ohms, go to s/m
   // *Y_cell = YX / (1 - R2 * YX) ; //1/ohms, go from S to ms
   //Serial.println(*Y_cell);
    *Y_cell = *Y_cell * slope + intercept; //calibrate

  }
  else {
    *Y_cell = YX * 1000;  //go to S/m
    //*Y_cell = YX;  //go from S/m to mS/cm

  }
  *YT_cell = tempcondtosal(*Y_cell * 10000, *T_cell); //convert mS/cm to uS/cm
  //  *YT_cell = tempcondtosal(*Y_cell, *T_cell); //convert mS/m to uS/cm

  //*Y_cell / (1 + alpha * (*T_cell - 25));
  return 1;
};

float CN0349::tempcondtosal(float cond, float temp) { //convert microsiemens to salinity, valid for 2 to 42 ppt
  float r, ds;
  float r2;
  float sal;

  const float a0 = 0.008;
  const float a1 = -0.1692;
  const float a2 = 25.3851;
  const float a3 = 14.0941;
  const float a4 = -7.0261;
  const float a5 = 2.7081;

  const float b0 = 0.0005;
  const float b1 = -0.0056;
  const float b2 = -0.0066;
  const float b3 = -0.0375;
  const float b4 = 0.0636;
  const float b5 = -0.0144;

  const float c0 = 0.6766097;
  const float c1 = 0.0200564;
  const float c2 = 0.0001104259;
  const float c3 = -0.00000069698;
  const float c4 = 0.0000000010031;

  if (temp < 0 || 30 < temp)
  {
    sal = -1;// "Out of range";
  }
  else {
    if (cond <= 0) {
      sal = -2;//"Out of range";
    }
    else {
      r = cond / 42914;
      r /= (c0 + temp * (c1 + temp * (c2 + temp * (c3 + temp * c4))));
      r2 = sqrt(r);
      ds = b0 + r2 * (b1 + r2 * (b2 + r2 * (b3 + r2 * (b4 + r2 * b5))));
      ds *= ((temp - 15.0) / (1.0 + 0.0162 * (temp - 15.0)));
      sal = a0 + r2 * (a1 + r2 * (a2 + r2 * (a3 + r2 * (a4 + r2 * a5)))) + ds;
      if (sal < 2.0) {
        sal = -3;//"Under scale";
      }
      else {
        if (sal > 42.0) {
          sal = -4;//"Over scale";
        }
      }
    }
  }
  return sal;
}

///////////////////////////////////////////////////////////////////////////////////////////
/////////ADG715
///////////////////////////////////////////////////////////////////////////////////////////
byte CN0349::ADG715CH(int channel) {   //checks channel numbers
  channel -= 1; //assume input is 1-8 -> 0-7
  if (channel < 0)
    channel = 0;
  else if (channel > 7)
    channel = 7;
  return channel;
};

//return status as a byte of all channel (1-8)
byte CN0349::ADG715read(byte channel) {  //if channel exceeds 9, read all
  byte value = 255; //error possibly?
  Wire.requestFrom(ADG715_ADDR, 0x01); //request one byte from address
  while (Wire.available())
    value = Wire.read(); //grab one byte
  if (channel < 9) //1-8
  {
    channel = ADG715CH(channel); //resize to 0-7
    value = bitRead(value, channel);
  }
  return value; //return all
};

void CN0349::ADG715writeChannel(byte channel, byte state) { //change status of a specified channel (1-8)
  byte value;
  value = ADG715read(9); //read all channels
  bitWrite(value, ADG715CH(channel), state);
  Wire.beginTransmission(ADG715_ADDR);
  Wire.write(value);
  Wire.endTransmission();
};

void CN0349::ADG715reset() { //clear out register
  Wire.beginTransmission(ADG715_ADDR);
  Wire.write(0x00); //clear out register
  Wire.endTransmission();
};
