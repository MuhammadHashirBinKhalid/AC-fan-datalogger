/*
Tasks remaining:
1. receive JSON and parse to have variables
2. Send timestamp = Done
3. Send Mac, password and local/device id = Done

Remember: Turning on debugging makes the watchdog timer to reset

//Working fine = 12/2/2019 v13

Resets WDT on RAM overflow


For RTC DS3231
VCC=3.3V
SCL=D1
SDA=D2
*/

#include <ArduinoJson.h>
#include <string.h>
#include "FS.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include "PietteTech_DHT.h"  // Uncommend if building using CLI
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRtimer.h>
#include <IRutils.h>
#include <EEPROM.h>
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>

//String getDateTime(const RtcDateTime& dt);

#ifndef UNIT_TEST
#include <Arduino.h>
#endif

#define TIME_SUBTRACTED 600000000
#define down_hex 0xFB38C7
#define up_hex 0xFBE817
#define onoff_hex 0xFBD22D
#define auto_hex 0xFBB04F

#define TESTFILE "/testfile.txt"
#define SEND_BYTE 200  //200 is good             //3 params & 24 readings = 1500 bytes
#define MAX_FILE_SIZE 3000    //000     //Add three zeros to make it 1,000,000=1MB
#define JSONbuffersizeval 1000
#define Testing 0                 // #TEMPORARY=1.. Otherwise it will be 0
#define AUTO_ENABLED 1  
#define sending_interval 10      //Seconds
//declaration
void dht_wrapper(); // must be declared before the lib initialization
bool datasend();
void sendIRdata(int digitToSend);
// Lib instantiate
void store_data(float temperature,float humidity);
void addtoJSON();
void Up(int level);
void Down(int level);
void setspeedto(int newspeed);

#define DHTPIN D4           // Digital pin for communications
uint16_t RECV_PIN = D5;       //For TSOP at pin 14
const int pirPin = D3;               // choose the input pin (for PIR sensor) at pin 0
const int OutputPin = D8;
const int IncbuttonPin = D6;     //Pin for Increment Button
const int DecbuttonPin = D7;   //Pin for Decrement Button

// An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU
// board).
IRrecv irrecv(RECV_PIN);
decode_results results;
unsigned long key_value = 0;
unsigned long filesize=0;
unsigned long sendposition=0;           //EEPROM variable
int teracounter=20;        //For testing
#define DHTTYPE  DHT11       // Sensor type DHT11/21/22/AM2301/AM2302
PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);
File f;
  
//raw codes for remote buttons
#define numberOfCodes 6
unsigned int IR_code[numberOfCodes][143]= {
{1322, 364,  1320, 364,  478, 1210,  1320, 364,  1320, 364,  1320, 364,  478, 1210,  478, 1208,  478, 1210,  1320, 366,  1320, 364,  478, 8102,  1342, 364,  1320, 364,  478, 1210,  1320, 366,  1320, 364,  1320, 366,  478, 1210,  478, 1210,  478, 1210,  1320, 364,  1330, 360,  478, 8094,  1344, 364,  1320, 366,  482, 1204,  1322, 364,  1320, 364,  1320, 364,  478, 1208,  478, 1210,  478, 1210,  1320, 364,  1320, 364,  478, 8122,  1320, 366,  1318, 364,  478, 1210,  1320, 364,  1320, 366,  1320, 364,  478, 1208,  478, 1210,  478, 1210,  1320, 366,  1320, 364,  478, 18212,  1342, 366,  1342, 342,  476, 1210,  1344, 340,  1320, 366,  1320, 366,  476, 1210,  476, 1212,  500, 1188,  1318, 366,  1320, 366,  476, 8102,  1342, 368,  1316, 366,  476, 1210,  1318, 366,  1320, 366,  1320, 366,  502, 1184,  476, 1212,  476, 1210,  1320, 366,  1318, 366,  476},  // UNKNOWN A4DA1D1F
{1344, 362,  1322, 364,  478, 1208,  1322, 364,  1322, 362,  480, 1208,  1322, 364,  480, 1208,  480, 1208,  480, 1208,  504, 1182,  480, 8102,  1344, 362,  1322, 362,  480, 1208,  1322, 362,  1322, 364,  480, 1208,  1322, 362,  506, 1182,  480, 1208,  480, 1208,  480, 1208,  480, 8100,  1344, 362,  1322, 364,  480, 1208,  1322, 362,  1322, 362,  480, 1208,  1322, 364,  478, 1208,  478, 1208,  506, 1182,  480, 1212,  474, 8100,  1346, 364,  1322, 362,  480, 1208,  1322, 364,  1322, 362,  480, 1208,  1322, 364,  480, 1206,  480, 1206,  480, 1208,  480, 1208,  480, 8100,  1344, 362,  1322, 362,  480, 1208,  1322, 362,  1322, 364,  480, 1206,  1322, 362,  480, 1206,  480, 1208,  480, 1208,  480, 1208,  480, 8098,  1346, 364,  1346, 338,  480, 1206,  1322, 362,  1322, 362,  480, 1208,  1322, 362,  480, 1206,  480, 1206,  480, 1208,  478, 1208,  480}, // UNKNOWN F2C316F3
{1344, 366,  1320, 364,  478, 1210,  1320, 364,  1320, 366,  476, 1210,  478, 1210,  478, 1210,  1320, 366,  476, 1210,  478, 1210,  478, 8100,  1344, 364,  1320, 366,  478, 1210,  1320, 364,  1320, 366,  478, 1208,  478, 1210,  478, 1210,  1320, 364,  478, 1210,  478, 1210,  478, 8124,  1320, 364,  1320, 370,  478, 1204,  1320, 366,  1320, 366,  476, 1210,  476, 1210,  478, 1210,  1320, 366,  478, 1210,  478, 1210,  478, 8102,  1342, 366,  1320, 364,  478, 1210,  1320, 366,  1320, 366,  476, 1210,  502, 1186,  478, 1210,  1320, 366,  476, 1210,  476, 1210,  478, 8100,  1344, 366,  1320, 364,  478, 1210,  1320, 366,  1320, 366,  476, 1210,  476, 1210,  476, 1210,  1320, 366,  478, 1210,  478, 1210,  476, 8104,  1342, 366,  1320, 366,  478, 1210,  1320, 366,  1318, 366,  478, 1210,  478, 1210,  478, 1210,  1320, 364,  478, 1210,  476, 1210,  478},  // UNKNOWN 4648B033
{1344, 362,  1322, 362,  480, 1208,  1322, 364,  1322, 362,  504, 1182,  480, 1208,  480, 1208,  480, 1208,  1322, 364,  480, 1206,  480, 8098,  1348, 364,  1322, 362,  480, 1208,  1322, 364,  1322, 362,  480, 1208,  480, 1206,  506, 1182,  478, 1208,  1322, 362,  480, 1206,  480, 8122,  1322, 364,  1322, 362,  480, 1208,  1322, 362,  1322, 362,  480, 1206,  480, 1206,  480, 1208,  480, 1208,  1322, 362,  480, 1208,  508, 8070,  1344, 362,  1348, 338,  478, 1208,  1322, 362,  1322, 362,  480, 1206,  480, 1208,  480, 1208,  504, 1182,  1322, 362,  480, 1208,  480, 8098,  1346, 362,  1322, 364,  478, 1208,  1322, 362,  1348, 336,  480, 1208,  480, 1208,  480, 1208,  478, 1208,  1348, 336,  504, 1182,  480, 8100,  1346, 362,  1322, 362,  478, 1208,  1322, 362,  1322, 364,  480, 1208,  478, 1208,  478, 1208,  504, 1182,  1322, 362,  480, 1208,  480},  // UNKNOWN 20D14BBB
{1320, 366,  1320, 366,  478, 1210,  1320, 366,  1320, 366,  1320, 364,  478, 1210,  476, 1210,  478, 1210,  478, 1210,  1320, 366,  1320, 7256,  1344, 366,  1320, 364,  478, 1208,  1320, 366,  1318, 366,  1320, 364,  478, 1210,  478, 1210,  478, 1210,  478, 1210,  1320, 364,  1320, 7258,  1342, 366,  1320, 366,  478, 1210,  1324, 364,  1314, 366,  1318, 366,  502, 1186,  476, 1210,  478, 1210,  476, 1210,  1320, 366,  1320, 7258,  1342, 366,  1320, 366,  476, 1210,  1318, 366,  1320, 366,  1318, 366,  478, 1210,  476, 1210,  478, 1210,  478, 1210,  1344, 340,  1318, 7278,  1320, 366,  1320, 364,  478, 1210,  1320, 366,  1344, 342,  1320, 364,  478, 1210,  478, 1210,  476, 1210,  476, 1210,  1344, 340,  1318, 7280,  1320, 366,  1320, 366,  476, 1210,  1318, 366,  1320, 366,  1320, 364,  476, 1210,  478, 1210,  478, 1210,  478, 1210,  1320, 364,  1320},  // UNKNOWN 1DA77DD8
{1340, 366,  1318, 366,  476, 1210,  1320, 366,  1318, 366,  476, 1210,  476, 1212,  1318, 366,  476, 1210,  502, 1184,  476, 1210,  476, 8102,  1344, 366,  1318, 366,  476, 1210,  1342, 342,  1320, 366,  476, 1210,  502, 1186,  1318, 366,  476, 1210,  476, 1210,  476, 1210,  476, 8124,  1346, 340,  1318, 366,  482, 1206,  1318, 366,  1320, 366,  476, 1210,  476, 1210,  1320, 366,  476, 1212,  476, 1210,  476, 1210,  478, 8104,  1340, 366,  1318, 366,  476, 1210,  1318, 366,  1318, 366,  476, 1212,  476, 1210,  1318, 366,  476, 1212,  502, 1186,  476, 1212,  480, 8098,  1342, 366,  1318, 366,  476, 1212,  1318, 366,  1318, 366,  476, 1210,  476, 1210,  1318, 366,  476, 1210,  476, 1210,  476, 1210,  476, 8124,  1320, 366,  1318, 366,  476, 1210,  1320, 366,  1318, 366,  476, 1210,  476, 1212,  1318, 366,  476, 1212,  476, 1210,  476, 1210,  476}  // UNKNOWN 4F7BE2FB
};

ESP8266WiFiMulti WiFiMulti;
RtcDS3231<TwoWire> Rtc(Wire);

long currentseconds = 0, previousseconds = 0;
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;                    // variable for reading the pin status
bool spiffsActive = false;
bool auto_mode=0;

String JSONstringBuffer;
//unsigned long currenttime=1234567890;     //Useless variable when RTC is connected
int myspeed;

 
void setup() {
   Serial.begin(115200);
  EEPROM.begin(128);
//   EEPROM.put(0,0);
//EEPROM.commit();
 EEPROM.get(6,myspeed);
if(myspeed>20) { Serial.print("Speed is greater than 20... Setting to 0"); EEPROM.put(6,myspeed=0);}
  EEPROM.get(0,sendposition);
  Serial.print("Send position loaded="); Serial.println(sendposition);
  Serial.print("My speed loaded="); Serial.println(myspeed);
  //Initialize SPIFFS
  if (SPIFFS.begin()) {
      Serial.println("SPIFFS Active");
      Serial.println();
      spiffsActive = true;
  } else {
      Serial.println("Unable to activate SPIFFS");
  }
  
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
Serial.println();
Serial.print("Compiled at=");
//Serial.println(compiled.TotalSeconds());
//SPIFFS.remove(TESTFILE);                        //Remove the text file... #TEMPORARY
   
  irrecv.enableIRIn();  // Start the receiver
  pinMode(OutputPin, OUTPUT);
  pinMode(IncbuttonPin,INPUT_PULLUP);
  pinMode(DecbuttonPin, INPUT_PULLUP);
 
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  pinMode(pirPin, INPUT);     // declare sensor as input
  WiFi.mode(WIFI_STA);
//  WiFiMulti.addAP("IC", "neduet33iacc44&");
  WiFiMulti.addAP("VPN IC", "abcd1234");
      WiFiMulti.addAP("ELD-PA OFFICE","neduet33iacc44&");
      WiFiMulti.addAP("APNA_NET_LAGWAO","howareyou?");  
  //  WiFiMulti.addAP("Samad","67643283");
  //   WiFiMulti.addAP("NEDUET-WiFi Services Dept.","");
Serial.println("Starting Yo");
  previousseconds = millis() / 1000;
  currentseconds = millis() / 1000;

//Now initialize RTC
  if (!Rtc.IsDateTimeValid()) 
    {
        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }

    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 
//     sendIRdata(myspeed);   
}

void dht_wrapper() {
  DHT.isrCallback();
}

bool motion = 0;
char charrec;
int result, temperature, humidity;
int h=0,l=0;
bool sentflag=1;
void loop() {

  val = digitalRead(pirPin);  // read input value
  if (val == HIGH) {            // check if the input is HIGH
    h++; l=0;
    if (pirState == LOW) {
      // we have just turned on
      Serial.println("Motion detected!");
      motion = 1;
      pirState = HIGH;
    }
  } else {h=0;l++;
    if (pirState == HIGH) {
      // we have just turned of
      Serial.println("Motion ended!");
      // We only want to print on the output change, not state
      pirState = LOW;
    }
  }

  if (irrecv.decode(&results)) {
    Serial.println("\n Ran decode:");  serialPrintUint64(results.value, HEX);     //receiving HEX code for ir remote buttons
    if (results.value == 0XFFFFFFFF)
      results.value = key_value;     
     
    irrecv.resume();  // Receive the next value
  
 if (results.value == down_hex) {         //HEX code for level 1
     Down(1);          auto_mode=0;                           //function call for button 1
    myspeed--;
    if(auto_mode){ auto_mode=0; myspeed=myspeed-16;}
    if(myspeed<0) myspeed=9;    
    EEPROM.put(6,myspeed);
    EEPROM.commit();
    }
    else if (results.value == up_hex) {     //HEX code for level 2
    Up(1); 
    myspeed++;  
    if(auto_mode){ auto_mode=0; myspeed=myspeed-16;}
     if(myspeed>9) myspeed=0;
     EEPROM.put(6,myspeed);
    EEPROM.commit();
    }
    else if (results.value == auto_hex) {     //HEX code for level 3
     if(AUTO_ENABLED)  auto_mode=1;                            //function call for button 3
    }
    else if (results.value == onoff_hex) {     //HEX code for level 4
     setspeedto(myspeed=0);          auto_mode=0;                              //function call for button 4
    }

//  delay(100);
  }
  currentseconds = millis() / 1000;
  if (currentseconds >= previousseconds + sending_interval) {
   if(!Testing){ result = DHT.acquireAndWait(0);
    temperature = DHT.getCelsius();
    humidity = DHT.getHumidity();}
  else{temperature=teracounter++; humidity=sendposition;}
  if(auto_mode){
 if(temperature>29) {if(myspeed!=17) setspeedto(myspeed=17);}
 else if(temperature>28) {if(myspeed!=18) setspeedto(myspeed=18);}
 else if(temperature>27) {if(myspeed!=19) setspeedto(myspeed=19);}
 else if(temperature>26) {if(myspeed!=20) setspeedto(myspeed=20);}
 else if(temperature>25) {if(myspeed!=21) setspeedto(myspeed=21);}
 else if(temperature>24) {if(myspeed!=22) setspeedto(myspeed=22);}
 else if(temperature>23) {if(myspeed!=23) setspeedto(myspeed=23);}
 else if(temperature>22) {if(myspeed!=24) setspeedto(myspeed=24);}
 else {if(myspeed!=21) setspeedto(myspeed=25);}
 
  }

  store_data(temperature, humidity);

 if(((filesize-sendposition)>SEND_BYTE)||(!sentflag)){
addtoJSON();

  }
  if (!Rtc.IsDateTimeValid())    {         Serial.println("RTC lost confidence in the DateTime!");  }   // Common Cuase: the battery on the device is low or even missing and the power line was disconnected
    previousseconds = currentseconds;
  }
}
