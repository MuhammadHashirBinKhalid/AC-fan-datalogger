void sendIRdata(int digitToSend){
    Serial.println();
    Serial.print("Digit to Send= ");
    Serial.println(digitToSend);
 EEPROM.put(6,digitToSend);
    EEPROM.commit();
if(digitToSend>5){digitToSend=digitToSend-8;}    
  for (int i = 0; i < 143; i++) {
    if (i % 2) { //Set Mark
      digitalWrite(OutputPin, HIGH);
      delayMicroseconds( IR_code[digitToSend][i] );
    }
    else {
      digitalWrite(OutputPin, LOW);
      delayMicroseconds( IR_code[digitToSend][i]);
    }
  }
}
