/*
  CN0349.cpp
*/

#include "Arduino.h"
#include "avr/pgmspace.h"
#include "NLSWDK.h"

///////////////////////////////////////////////////////////////////////////////////////////
//Nimblelink Skywire Development Kit  (Key functions: modemSetup, sendData, getExoData, signalStrength)
//NL_SWDK
//NL_SW_LTE_GELS3
///////////////////////////////////////////////////////////////////////////////////////////

void NLSWDK::modemSetup() {    //setup modem power wise and software wise
  String modemResponse = ""; // Modem responce to a command
  // initialize serial debug communication with PC over USB connection
  Debug.begin(115200);
  while (!Debug) ; // wait for serial debug port to connect to PC
  for (int q = 5; q > 0; q--) {
    Debug.println(q, DEC);
    delay(250);
  } 
  Debug.println("Socket Dial To Send data to Exosite");
  // Start cellular modem
  Debug.println("Starting Cellular Modem");
  //Turn off internal PU resistor Arduino I/O pin 12 is connected to modem ON_OFF signal.
  digitalWrite(12, LOW); //ON_OFF has internal 200k pullup resistor and needs to be driven low for >1s, don't accept default

  // Configure I/O pin 12 as output
  pinMode(12, OUTPUT);
  pinMode(10, OUTPUT);  // SW_DTR pin (must be driven low if unused)
  pinMode(11, OUTPUT);  // SW_RTS pin (must be driven low if unused)
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
  digitalWrite(12, SW_ON);  // Turn on Skywire modem
  delay(1100); // modem requires >1s pulse
  pinMode(12, INPUT);  // Return I/O pin 12 to input/hi-Z state
  delay(30100); // Wait > 30 seconds for initialization

  // Initialize serial port to communicate with modem
  Debug.println("Initializing modem COM port");
  SW_Serial.begin(115200);
  while (!SW_Serial) ;  // send "AT" command to confirm modem is turned on
  Debug.println("Test AT command");
  WaitForResponse("AT\r", "OK", 100, modemResponse);
  // turn on echo for Cat 1 modem
  WaitForResponse("ATE1\r", "OK", 500, modemResponse);
  Debug.println("Reseting modem");
  WaitForResponse("AT+SOFTRESET\r", "OK", 500, modemResponse);  // Soft reset of modem
  WaitForResponse("AT+CEREG=0\r", "OK", 500, modemResponse);  // turn off URC (unsolicited result code) messages for Cat 1 modem
  WaitForResponse("AT+CMEE=2\r", "OK", 1000, modemResponse);  // turn on verbose error messages
// Setup PDP context
  Debug.println("Setting up PDP context");
  SendModemCommand("AT^SISC=0\r", "OK", 500, modemResponse);
  WaitForResponse("AT^SISS=0,\"srvType\",\"Socket\"\r", "OK", 500, modemResponse);
  WaitForResponse("AT^SISS=0,\"conId\",3\r", "OK", 500, modemResponse); // Set to use PDP context 3
  WaitForResponse("AT^SISS=0,\"address\",\"socktcp://m2.exosite.com:80\"\r", "OK", 500, modemResponse); // Configure socket to use TCP on port 80
  delay(10000);
  WaitForResponse("AT+CSQ\r", "OK", 500, modemResponse);  // Check signal strength
  WaitForResponse("AT+CGMR\r", "OK", 500, modemResponse);  // send command to modem to get firmware version
  String modemResp = "";  // activate PDP context
  delay(2000);
}

String NLSWDK::signalStrength() { //grab the signal strength
    String modemResponse = "";
    WaitForResponse("AT+CSQ\r", "OK", 500, modemResponse);  // Check signal strength
    modemResponse.remove(17);
    modemResponse.remove(0, 15);
    int dbm = 2 * (modemResponse.toFloat() - 1) - 113; //convert to dBm to cellular bars
    if (dbm * -1 > 117) {
      modemResponse = String(dbm) + " (0 bars)";
    }
    else if (dbm * -1 <= 117 && dbm * -1 > 110) {
      modemResponse = String(dbm) + " (1 bar)";
    }
    else if (dbm * -1 <= 110 && dbm * -1 > 100) {
      modemResponse = String(dbm) + " (2 bars)";
    }
    else if (dbm * -1 <= 100 && dbm * -1 > 90) {
      modemResponse = String(dbm) + " (3 bars)";
    }
    else if (dbm * -1 <= 90) {
      modemResponse = String(dbm) + " (4 bars)";
    }
    return modemResponse;
  }

void NLSWDK::sendData(String dataString, String CIK) {  // Transmits data to exosite
    String modemResponse = "";
    // Setup HTTP connection to exosite
    Debug.println("Setting up connection");
  //  SendModemCommand("AT^SISC=0\r", "OK", 500, modemResponse); // Close connection
    WaitForResponse("AT^SICA=1,3\r", "OK", 500, modemResponse);
    WaitForResponse("AT^SISO=0\r", "OK", 500, modemResponse);
    int len = dataString.length();
    SendModemCommand("AT^SISW=0,375\r", "^SISW: 0,375,0", 500, modemResponse); // Set command length to total number of characters of the POST command http_command
    String http_command;

    //Debug.println("Sending data to exosite");

    // Build string to send to exosite
    http_command = "POST /onep:v1/stack/alias HTTP/1.1\n"; //end data to exosite with post command
    http_command += "Host: m2.exosite.com\n";
    http_command += "X-Exosite-CIK: " + CIK + "\n";
    http_command += "Content-Type: application/x-www-form-urlencoded; charset=utf-8\n";
    http_command += "Accept: application/xhtml+xml\n";
    http_command += "Content-Length: " + String(len) + "\n\n"; // Set content length to number of characters in next line
    http_command += dataString;
    //Debug.print(http_command);
    SW_Serial.print(http_command);
    delay(5000);
    while (PrintModemResponse() > 0);
    // Could put code here to check for data transmission error "+CME ERROR"
    WaitForResponse("AT^SISC=0\r", "OK", 500, modemResponse); // Close connection
    // Print output
    Debug.println("\nInformation sent to exosite.");
  }
  


String NLSWDK::getExoData(String alias, String CIK) {  //get data alias from exosite
    /*/////////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
    /*/////////////////////////////////////////*VERY IMPORTANT NOTE*\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
    /*/////////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
    /*
       NOTE: In order to receive the entire HTTP GET message, you must adjust the size of the Arduino RX buffer.
       The file is located at:

       C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\HardwareSerial.h

       Edit this file as administrator, changing the value in the line:

       #define SERIAL_RX_BUFFER_SIZE 64

       to a larger value, such as:

       #define SERIAL_RX_BUFFER_SIZE 512

       Keep in mind the size of your program and the size of the information you are receiving when selecting
       your value!

       This example has been tested as working with a value of 512 bytes on an Arduino Leonardo R3
    */
    /*/////////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
    /*/////////////////////////////////////////*VERY IMPORTANT NOTE*\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
    /*/////////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
    String modemResponse = "";
    // Setup HTTP connection to exosite
    SendModemCommand("AT^SISC=0\r", "OK", 500, modemResponse); // Close connection
    WaitForResponse("AT^SICA=1,3\r", "OK", 500, modemResponse);
    WaitForResponse("AT^SISO=0\r", "OK", 500, modemResponse);
    SendModemCommand("AT^SISW=0,250\r", "^SISW: 0,250,0", 500, modemResponse);  // Set command length to total number of characters of the POST command http_command

    String http_command;
    Debug.println("Getting data from exosite");
    // Build string to send to exosite
    http_command = "GET /onep:v1/stack/alias?" + alias + " HTTP/1.1\n"; //end data to exosite with GET command
    http_command += "Accept: application/x-www-form-urlencoded; charset=utf-8\n";
    http_command += "Accept-Encoding: gzip, deflate\n";
    http_command += "Host: m2.exosite.com\n";
    http_command += "Connection: Close\n";
    http_command += "X-Exosite-CIK: " + CIK + "\n\n\n";
    Debug.print(http_command);
    SendModemCommand(http_command, "OK", 500, modemResponse);
	//Debug.println("modem1"+modemResponse);
    SendModemCommand(http_command, "OK", 500, modemResponse);
    //Debug.println("modem2"+modemResponse);

    WaitForResponse("AT^SISR=0,1000\r", "^SISR: 0,", 500, modemResponse);
	//Debug.println("modem3"+modemResponse);
    String dataResponse = modemResponse;

	WaitForResponse("AT^SISR=0,1000\r", "^SISR: 0,", 500, modemResponse);
	//Debug.println("modem4"+modemResponse);

    dataResponse.remove(0, 138);
    while (PrintModemResponse() > 0);
    // Could put code here to check for data transmission error "+CME ERROR"
    WaitForResponse("AT^SISC=0\r", "OK", 500, modemResponse); // Close connection*/
    // Print output
    Debug.println("\nInformation taken from exosite.");
    return dataResponse;
  }



  // sends a command to the modem, waits for the specified number of milliseconds,
  // checks whether the modem response contains the expected response, and
  // appends the remaining response characters to the out parameter respOut
  // returns true if command received the expected response
  
bool NLSWDK::SendModemCommand(String command, String expectedResp, int msToWait, String& respOut) {
    int cmd_timeout = 0;
    SW_Serial.print(command);
    delay(msToWait);

    // wait for data to become available, but timeout eventually if no response is received
    while (!SW_Serial.available()) {
      cmd_timeout++;
      if (cmd_timeout == 1000) {
        Debug.println("command timeout");
        return false;
      }
      delay(10);
    }

    // read response from modem
    String resp = "";
    respOut = "";
    while (SW_Serial.available() > 0) {
      resp += char(SW_Serial.read());
      if (resp.endsWith(expectedResp)) {
        respOut = resp;
        while (SW_Serial.available() > 0)
          respOut += char(SW_Serial.read());  // append remaining response characters (if any)
        return true;
      }
    }
    respOut = resp;
    return false;
  }

void NLSWDK::ConsumeModemResponse() { // empty read buffer
    while (SW_Serial.available())
      SW_Serial.read();
  }

  // repeatedly sends command to the modem until correct response is received
void NLSWDK::WaitForResponse(String command, String expectedResp, int msToWait, String& respOut) {
    bool isExpectedResp;
    do {
      isExpectedResp = SendModemCommand(command, expectedResp, msToWait, respOut);
      Debug.println(respOut);
      SW_Serial.flush();        // just in case any characters weren't transmitted
      ConsumeModemResponse();   // just in case any characters remain in RX buffer
    }
    while (!isExpectedResp);
  }

String NLSWDK::GetModemResponse() {// returns modem response as a String
    String resp = "";
    while (SW_Serial.available() > 0) {
      resp += char(SW_Serial.read());
    }
    return resp;
  }

int NLSWDK::PrintModemResponse() {// consumes and prints modem response
    String resp = "";
    while (SW_Serial.available() > 0) {
      // read incoming modem response into temporary string
      resp += char(SW_Serial.read());
    }
    Debug.println(resp);
    //return number of characters in modem response buffer -- should be zero, but some may have come in since last test
    return SW_Serial.available();
  }