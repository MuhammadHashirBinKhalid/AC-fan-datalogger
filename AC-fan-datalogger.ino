/*

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
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
//#include "PietteTech_DHT.h"  // Uncommend if building using CLI
#include "DHTesp.h"
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRtimer.h>
#include <IRutils.h>
#include <EEPROM.h>
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
#include <Ticker.h>
//String getDateTime(const RtcDateTime& dt);

#ifndef UNIT_TEST
#include <Arduino.h>
#endif

#define TIME_SUBTRACTED 600000000
#define down_hex 0xFB38C7
#define up_hex 0xFBE817
#define onoff_hex 0xFBD22D
#define auto_hex 0xFBB04F

//#define TESTFILE "/testfile2.txt"
#define FILE1 "/testfile3.txt"
#define FILE2 "/testfile4.txt"
#define FILE3 "/testfile5.txt"
//#define OLDTESTFILE "/testfile.txt"
#define SEND_BYTE 20000  //200 is good             //3 params & 24 readings = 1500 bytes
//#define MAX_FILE_SIZE 3000    //000     //Add three zeros to make it 1,000,000=1MB
#define JSONbuffersizeval 1000
#define Testing 0                 // #TEMPORARY=1.. Otherwise it will be 0
#define AUTO_ENABLED 1
#define sending_interval 30      //Seconds
//declaration
bool datasend(String);
void sendNEC(unsigned long data, int nbits);
// Lib instantiate
void store_data(float temperature, float humidity);
//void addtoJSON();
void resetdht();
String get_File(int thisfilenumber);
// Lib instantiate
void Up(int level);
void Down(int level);
void setspeedto(int newspeed);
void mark(int time); void space(int time);
#define DHTPIN 2           // Digital pin for communications
uint16_t RECV_PIN = D5;       //For TSOP at pin 14
const int pirPin = D6;               // choose the input pin (for PIR sensor) at pin 0
const int OutputPin = D7;

// An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU
// board).
IRrecv irrecv(RECV_PIN);
decode_results results;
unsigned long key_value = 0;
unsigned long filesize = 0;
unsigned long sendposition = 0;         //EEPROM variable
int teracounter = 20;      //For testing
#define DHTTYPE  DHT11       // Sensor type DHT11/21/22/AM2301/AM2302
//PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);
DHTesp dht;
File f;

// NEC pulse parameters in usec
#define NEC_HDR_MARK  9000   //Each code sequence starts with a 9ms pulse
#define NEC_HDR_SPACE 4500  //followed by a 4.5ms silence/space
#define NEC_BIT_MARK  560       //Logical '0' – a 562.5µs pulse burst followed by a 562.5µs space, with a total transmit time of 1.125ms while Logical '1' – a 562.5µs pulse burst followed by a 1.6875ms space, with a total transmit time of 2.25ms
#define NEC_ONE_SPACE 1600   //total of two logics
#define NEC_ZERO_SPACE  560  //Final 562.5µs pulse burst to show end of message transmission.
#define NEC_RPT_SPACE 2250  //A 2.25ms space for repeat code
#define TOPBIT 0x80000000


RtcDS3231<TwoWire> Rtc(Wire);

long currentseconds = 0, previousseconds = 0;
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;                    // variable for reading the pin status
bool spiffsActive = false;
bool auto_mode = 0;

String JSONstringBuffer;
//unsigned long currenttime=1234567890;     //Useless variable when RTC is connected
int myspeed;
int file_number = 1;
int send_file = 1;
String TESTFILE = "/testfile3.txt";
String header_string="";


//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}
void setup() {

  dht.setup(DHTPIN, DHTesp::DHTTYPE); // Connect DHT sensor to GPIO 17
  delay(1000); // delay necessary after power up for DHT to stabilize
  Serial.begin(115200);
  EEPROM.begin(128);
  EEPROM.put(0, send_file);
  //EEPROM.put(6,0);
  EEPROM.put(12, file_number = 1);
  EEPROM.commit();
  //   EEPROM.put(0,0);
  //EEPROM.commit();
  EEPROM.get(6, myspeed);
  EEPROM.get(12, file_number);
  if (myspeed > 20) {
    Serial.print("Speed is greater than 20... Setting to 0");
    EEPROM.put(6, myspeed = 0);
  }
  EEPROM.get(0, send_file);
  Serial.print("Sent file loaded="); Serial.println(send_file);
  Serial.print("Send position loaded="); Serial.println(sendposition);
  Serial.print("My speed loaded="); Serial.println(myspeed);
  mark(NEC_BIT_MARK);//to initialize OutputPin
  space(0);
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
  Serial.println(compiled.TotalSeconds());
 // SPIFFS.remove(TESTFILE);                        //Remove the text file... #TEMPORARY
 // SPIFFS.remove(FILE1);                        //Remove the text file... #TEMPORARY
 //SPIFFS.remove(FILE2);                        //Remove the text file... #TEMPORARY
 // SPIFFS.remove(FILE3);                        //Remove the text file... #TEMPORARY
//  SPIFFS.remove(OLDTESTFILE);                        //Remove the text file... #TEMPORARY
 // SPIFFS.remove(TESTFILE);                        //Remove the text file... #TEMPORARY

  irrecv.enableIRIn();  // Start the receiver
  pinMode(OutputPin, OUTPUT);
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  pinMode(pirPin, INPUT);     // declare sensor as input

   //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

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
  Serial.print("Mac address= ");
  Serial.println(WiFi.macAddress());
header_string="{\"return\":0,\"mac\":\"";
header_string+=String(WiFi.macAddress());
//header_string+="5C:CF:7F:ED:D0:A7";                              
header_string+="\",\"password\":123456,\"localId\":1,\"params\":[\"timestamp\",\"temperature\",\"humidity\",\"ms\"],\"values\":[";

}

bool motion = 0;
char charrec;
int result, temperature, humidity;
int h = 0, l = 0;
bool sentflag = 1;
void loop() {

  val = digitalRead(pirPin);  // read input value
  if (val == HIGH) {            // check if the input is HIGH
    h++; l = 0;
    if (pirState == LOW) {
      // we have just turned on
      Serial.println("Motion detected!");
      motion = 1;
      pirState = HIGH;
    }
  } else {
    h = 0; l++;
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
      Down(1);          auto_mode = 0;                         //function call for button 1
      myspeed--;
      if (auto_mode) {
        auto_mode = 0;
        myspeed = myspeed - 16;
      }
      if (myspeed < 0) myspeed = 9;
      EEPROM.put(6, myspeed);
      EEPROM.commit();
    }
    else if (results.value == up_hex) {     //HEX code for level 2
      Up(1);
      myspeed++;
      if (auto_mode) {
        auto_mode = 0;
        myspeed = myspeed - 16;
      }
      if (myspeed > 9) myspeed = 0;
      EEPROM.put(6, myspeed);
      EEPROM.commit();
    }
    else if (results.value == auto_hex) {     //HEX code for level 3
      if (AUTO_ENABLED)  auto_mode = 1;                         //function call for button 3
    }
    else if (results.value == onoff_hex) {     //HEX code for level 4
      setspeedto(myspeed = 0);          auto_mode = 0;                          //function call for button 4
    }
    //  delay(100);
  }
  currentseconds = millis() / 1000;
  if (currentseconds >= previousseconds + sending_interval) {
    if (!Testing) { //result = DHT.acquireAndWait(0);
      temperature = dht.getTemperature();
      humidity = dht.getHumidity();
      if ((humidity > 500) || (temperature > 500)) {
        Serial.println();
        Serial.println("DHT is doing issue... Resetting it");
        //   resetdht();
        temperature = dht.getTemperature();
        humidity = dht.getHumidity();
      }
      //  Serial.println("DHT worked");
    }
    else {
      temperature = teracounter++;
      humidity = sendposition;
    }
    if (auto_mode) {
      if (temperature > 45) {
        if (myspeed != 17) setspeedto(myspeed = 17);      //Speed 1-fastest
      }
      else if (temperature > 43) {
        if (myspeed != 18) setspeedto(myspeed = 18);       //Speed 2
      }
      else if (temperature > 37) {
        if (myspeed != 19) setspeedto(myspeed = 19);       //Speed 3
      }
      else if (temperature > 35) {
        if (myspeed != 20) setspeedto(myspeed = 20);       //Speed 4
      }
      else if (temperature > 30) {
        if (myspeed != 21) setspeedto(myspeed = 21);       //Speed 5
      }
      else if (temperature > 25) {
        if (myspeed != 22) setspeedto(myspeed = 22);       //Speed 6
      }
      else if (temperature > 20) {
        if (myspeed != 23) setspeedto(myspeed = 23);       //Speed 7
      }
      else if (temperature <20) {
        if (myspeed != 24) setspeedto(myspeed = 24);       //Speed 8
      }
       else if (temperature <15) {
        if (myspeed != 25) setspeedto(myspeed = 25);       //Speed 9-slowest
      }
    }
    store_data(temperature, humidity);
    Serial.print("File number is =");
    Serial.println(file_number);
    Serial.print("Send File number is =");
    Serial.println(send_file);
    if (file_number != send_file) {
      bool sentflag = datasend(get_File(send_file));
      if (sentflag) {
        if (send_file == 3) {
          SPIFFS.remove(get_File(3));
          Serial.println("Removing file 3");
          send_file = 1;
        }
        else send_file++;
        EEPROM.put(0, send_file); EEPROM.commit();
      }
    }
    if (filesize > SEND_BYTE) {
      file_number++;
      if ((file_number > 3) && (send_file >= 3)) {
        SPIFFS.remove(get_File(1));
        Serial.println("Removing file 1");
        SPIFFS.remove(get_File(2));
        Serial.println("Removing file 2");
        file_number = 1;
      }
      TESTFILE = get_File(file_number);
      Serial.print("Initiated new TESTFILE=");
      Serial.println(TESTFILE);
      EEPROM.put(12, file_number);
      EEPROM.commit();
    }

    if (!Rtc.IsDateTimeValid())    {
      Serial.println("RTC lost confidence in the DateTime!");   // Common Cuase: the battery on the device is low or even missing and the power line was disconnected
    }
    previousseconds = currentseconds;
  }
}
