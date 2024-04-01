/*
  NLSWDK.h
*/
#ifndef NLSWDK_h
#define NLSWDK_h

#include "Arduino.h"
#include <Wire.h>// used for I2C communication
#include "avr/pgmspace.h"
#include<HardwareSerial.h> // for Serial Logging
#define Debug Serial //define Serial ports for Arduino Mega
#define SW_Serial Serial1  //bend pins out from Serial0 and wire to Serial3 for Arduino Mega
#define SW_ON LOW

class NLSWDK{
	public:
		void modemSetup();  //setup modem power wise and software wise
		String signalStrength(); //grab the signal strength
		void sendData(String dataString, String CIK);  // Transmits data to exosite
		String getExoData(String alias, String CIK); //get data alias from exosite
	private:
	    bool SendModemCommand(String command, String expectedResp, int msToWait, String& respOut); // sends a command to the modem
		void ConsumeModemResponse(); // empty read buffer
		void WaitForResponse(String command, String expectedResp, int msToWait, String& respOut);// returns modem response as a String
		String GetModemResponse();// returns modem response as a String
		int PrintModemResponse(); // consumes and prints modem response
		};
#endif
