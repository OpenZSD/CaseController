void setup() 
{
  Serial.begin(9600);
  pinMode(3, OUTPUT); //back upper L
  pinMode(5, OUTPUT); //base
  pinMode(6, OUTPUT); //back upper T
  pinMode(9, OUTPUT); //back lower

  setPwmFrequency(3, 32);
  setPwmFrequency(5, 8);
  setPwmFrequency(6, 8);
  setPwmFrequency(9, 8);

  Serial.print("CPU_LED\n");
}


void setPwmFrequency(int pin, int divisor)
{
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) 
  {
    switch(divisor) 
    {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) 
    {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    }
    else 
    {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } 
  else if(pin == 3 || pin == 11)
  {
    switch(divisor) 
    {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}


void loop() 
{
  if(Serial.available()) { Serial.print("CPU_LED\n"); }
  while(Serial.available()) { Serial.read(); }
  
  for(int i = 9; i < 211; i += 1)
  {    
    int k = i < 110 ? i : 220 - i;
    if(k < 10) { k = 10; }
    
    analogWrite(3, k*k/100);
    analogWrite(5, k*k/100);
    analogWrite(6, k*k/100);
    analogWrite(9, k*k/100);
    delay(150);
  }
}



