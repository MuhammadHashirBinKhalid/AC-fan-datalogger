//Created on 6/2/2019


void addtoJSON() {
  int temp_position;
  String one_reading;
  byte mac[6];
 //      WiFi.macAddress(mac);

  StaticJsonBuffer<JSONbuffersizeval> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  // Create the root object
  root["return"] = 0;
  root["mac"]= "5C:CF:7F:ED:D0:A7";   //WiFi.macAddress();
  root["password"]=123456;
  root["localId"]=1;
  JsonArray& params = root.createNestedArray("params"); //JSON array
  params.add("timestamp");
  params.add("temperature"); //send temperature
  params.add("humidity"); //send humidity
  params.add("ms"); //send speed of fan in last 3 bits and status of PIR in 1st bit

  JsonArray& values = root.createNestedArray("values"); //JSON array


  File f2 = SPIFFS.open(TESTFILE, "r");
  if (!f2) {
    Serial.println("Unable To Open file for Reading");
  }
  else {
    Serial.print("Contents of file '");
    Serial.print(TESTFILE);
    Serial.println("':");
    Serial.println();
    while (f2.position() < f2.size())             //DO THE FOLLOWING FOR COMPLETE FILE
    { one_reading = f2.readStringUntil('\n');
      one_reading.trim();
      Serial.println(one_reading);
      if (f2.position() > sendposition) {                 //Do this for part of file that has not been sent
        int ind1, ind2;
        ind2 = one_reading.indexOf(',');
        String timestring = one_reading.substring(0,ind2);
        ind1 = one_reading.indexOf(',', ind2 + 1);
        String tempstring = one_reading.substring(ind2+1, ind1);
        ind2 = one_reading.indexOf(',', ind1 + 1);
        String humstring = one_reading.substring(ind1 + 1, ind2);
        String thirdstring = one_reading.substring(ind2 + 1);
        int p_temp = tempstring.toInt();
        int p_hum = humstring.toInt();
        int p_third = thirdstring.toInt();
        unsigned long sendtime=timestring.toInt();
        JsonArray& array1 = jsonBuffer.createArray();
        array1.add(sendtime);
        array1.add(p_temp);
        array1.add(p_hum);
        array1.add(p_third);
        values.add(array1);
        temp_position = f2.position();
      }
          if((jsonBuffer.size())>(JSONbuffersizeval-200))break;
    }
    filesize = f2.size();
    f2.close();

    Serial.println();
    Serial.println();
    Serial.println("File ended");
    Serial.print("File size= ");
    Serial.print(filesize);
    Serial.print(", File temp pos:");
    Serial.println(temp_position);
    Serial.print(", File send pos:");
    Serial.println(sendposition);

//    root.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
JSONstringBuffer="";
    root.prettyPrintTo(JSONstringBuffer);
    root.printTo(Serial);
    Serial.println();
    Serial.print("JSON buffer size=");
    Serial.println(jsonBuffer.size());
    Serial.print("Size of JSON string Buffer=");
    Serial.println(JSONstringBuffer.length());
//    Serial.print("JSONstring buffer=");
//    Serial.println(JSONstringBuffer);

    sentflag = datasend();
    Serial.print("Sent flag=");
    Serial.println(sentflag);
    if (sentflag) {
      sendposition = temp_position;
      if (filesize > MAX_FILE_SIZE) {
        SPIFFS.remove(TESTFILE); Serial.println("Over size.. Removing file");
      } 
      else{
         EEPROM.put(0,sendposition);
        EEPROM.commit();
      }
    }
  }
}
