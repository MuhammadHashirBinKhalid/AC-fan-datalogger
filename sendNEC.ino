void sendNEC(unsigned long data, int nbits) {
  mark(NEC_HDR_MARK);
  space(NEC_HDR_SPACE);

  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      mark(NEC_BIT_MARK);
      space(NEC_ONE_SPACE);
    }
    else {
      mark(NEC_BIT_MARK);
      space(NEC_ZERO_SPACE);
    }
    data <<= 1;
  }
  mark(NEC_BIT_MARK);
  space(NEC_HDR_SPACE);
  mark(NEC_BIT_MARK);
  space(0);
}
//to output an inverted de-modulated signal
void mark(int time) {
  //inverting the signal
  digitalWrite(OutputPin, LOW);
  delayMicroseconds(time);     //digitalwrite removes the modulation to provide a demodulated signal
}
void space(int time) {
  digitalWrite(OutputPin, HIGH);
  delayMicroseconds(time);
}
