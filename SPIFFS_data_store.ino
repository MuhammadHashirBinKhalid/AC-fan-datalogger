void store_data(float temperature,float humidity) {
RtcDateTime now = Rtc.GetDateTime();
String mystr="";
mystr+="[";
if (!Rtc.IsDateTimeValid()) mystr+="0";
else mystr+=String(now.TotalSeconds()-TIME_SUBTRACTED); //getDateTime(now);//String(currenttime);
mystr+=",";
mystr+=String(temperature);
mystr+=",";
mystr+=String(humidity);
mystr+=",";
mystr+=String((motion<<7)|myspeed);
mystr+="]";
  if (motion == 1) { motion = 0;}
  if (spiffsActive) {
    if (SPIFFS.exists(TESTFILE)) {
      for(int i=0;i<=400;i++){           //loop to check data sending
      f = SPIFFS.open(TESTFILE, "a");}
      if (!f) {
        Serial.println("Unable To Open file");
      }
      else {
       
if((filesize<=SEND_BYTE+500)&&(file_number<=3)){
        Serial.print("Appending line to file:");
        mystr=","+mystr;
        Serial.println(mystr);
//     for(int bohot=0;bohot<1000;bohot++){
        f.println(mystr);
        }
        else{ Serial.print("Memory is full. Can not append line: ");
        Serial.println(mystr);
        }
//        }
if(f.size()!=filesize){
      filesize=f.size();
}
else if(filesize<=SEND_BYTE){
SPIFFS.remove(FILE1);                        //Remove the text file... #TEMPORARY
SPIFFS.remove(FILE2);                        //Remove the text file... #TEMPORARY
SPIFFS.remove(FILE3);                        //Remove the text file... #TEMPORARY

  }
      f.close();
      Serial.print("File size= ");
      Serial.println(filesize);
      Serial.println();
//      }
//      else{
//        Serial.println("Could not append following line to file... File is oversize");
//        Serial.println(mystr);
//      }
      }
    }
    else {
      Serial.print("Unable To Find file");
      Serial.println(".. Creating file");
      Serial.println();
      f = SPIFFS.open(TESTFILE, "w");
      if (!f) {
        Serial.println("file creation failed");
      }
      else {
        Serial.print("File creation successful");
        Serial.println(TESTFILE);
        sendposition=f.position();
   
        f.println(mystr);
       Serial.print("Appending 1st line:");
       Serial.println(mystr);
       filesize=f.size();
         f.close();
          EEPROM.put(0,sendposition);
          EEPROM.commit();
      }
    }
  }
}

void resetdht(){
  pinMode(DHTPIN, OUTPUT); // switches power to DHT on
     digitalWrite(DHTPIN, LOW); // sets output to gnd
  delay(1000); // delay necessary after power up for DHT to stabilize
  pinMode(DHTPIN, INPUT_PULLUP); // switches power to DHT on
  dht.setup(DHTPIN, DHTesp::DHTTYPE); // Connect DHT sensor to GPIO 17
delay(2000);
}
