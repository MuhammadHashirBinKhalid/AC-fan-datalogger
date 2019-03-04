//Storing pointers in Permanent memory is remaining

void store_data(float temperature,float humidity) {
   RtcDateTime now = Rtc.GetDateTime();
 
String mystr="";
if (!Rtc.IsDateTimeValid()) mystr+="0";
else mystr+=String(now.TotalSeconds()-TIME_SUBTRACTED); //getDateTime(now);//String(currenttime);
//mystr+="123456";
mystr+=",";
mystr+=String(temperature);
mystr+=",";
mystr+=String(humidity);
mystr+=",";
mystr+=String((motion<<7)|myspeed);
  if (motion == 1) { motion = 0;}
  if (spiffsActive) {
    if (SPIFFS.exists(TESTFILE)) {
      f = SPIFFS.open(TESTFILE, "a");
      if (!f) {
        Serial.println("Unable To Open file");
      }
      else {
        Serial.print("Appending line to file:");
        Serial.println(mystr);
        f.println(mystr);
      filesize=f.size();
      
      f.close();
      Serial.print("File size= ");
      Serial.println(filesize);
      Serial.println();
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
        filesize=f.size();
         f.close();
          EEPROM.put(0,sendposition);
          EEPROM.commit();
      }
    }
  }
}

//String getDateTime(const RtcDateTime& dt)
//{
// 
//    char datestring[20];
//
//    snprintf_P(datestring, 
//            countof(datestring),
//            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
//            dt.Month(),
//            dt.Day(),
//            dt.Year(),
//            dt.Hour(),
//            dt.Minute(),
//            dt.Second() );
//    Serial.print(datestring);
//    
//}
//
