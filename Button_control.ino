void setspeedto(int newspeed){
     Serial.print("Speed to Set= ");
    Serial.println(newspeed);
    int prevspeed;
    EEPROM.get(6,prevspeed);
    Serial.print("In set speed to: Prev speed was ");
    Serial.println(prevspeed);
 EEPROM.put(6,newspeed);
    EEPROM.commit();
if(newspeed>9){newspeed=newspeed-16;}    
if(prevspeed>9){prevspeed=prevspeed-16;}
int diff_speed=newspeed-prevspeed;
Serial.print("Diff in Speed =");
Serial.println(diff_speed);
if(diff_speed>0){ Up(diff_speed);}
else if(diff_speed<0){Down(0-diff_speed);}
}

void Up(int level)
{
  //OUTPUT loop for INC Button
  Serial.print("\n Speed Up by : ");
  Serial.println(level);
  for (int j = 0; j < level;) {
    pinMode(IncbuttonPin , OUTPUT);
    digitalWrite(IncbuttonPin , LOW);
    delay(200);
    digitalWrite(IncbuttonPin , HIGH);
    j++; //Serial.print("\t Up "); Serial.print(j);
    delay(200);
  }
  pinMode(IncbuttonPin , INPUT_PULLUP);
}
void Down(int level)
{ //OUTPUT loop for DEC Button
  Serial.print("\n Speed Down by: ");
  Serial.println(level);
  for (int i = 0; i < level;) {
    pinMode(DecbuttonPin, OUTPUT); delay(10);
    digitalWrite(DecbuttonPin, 0);
    delay(200);
    digitalWrite(DecbuttonPin, 1);
    i++; //Serial.print("\t Down: "); Serial.print(i);
    delay(200);
  }
  pinMode(DecbuttonPin, INPUT_PULLUP);
}
