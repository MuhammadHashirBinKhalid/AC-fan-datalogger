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
  for (int i = 0; i < 3; i++) {
   sendNEC(0xFBE817, 32);   delay(40);
  }
  delay(100); //0.1 second delay between each signal burst
  j++;}//datasend();
}
void Down(int level)
{ //OUTPUT loop for DEC Button
  Serial.print("\n Speed Down by: ");
  Serial.println(level);
  for (int j = 0; j < level;) {
  for (int i = 0; i < 3; i++) {
   sendNEC(0xFB38C7, 32);   delay(40);
  }
  delay(100); //0.1 second delay between each signal burst
  j++;} 
}
