int sTicker;
int sBeacon;
int sTickDur;
char sMode;

#define PULSEM 0
#define PULSEXPM 1
#define BREATHE 2
#define HBREATHE 3

void setup()
{
   pinMode(7, OUTPUT);
   pinMode(8, OUTPUT);
   pinMode(9, OUTPUT);

   digitalWrite(7, LOW);
   digitalWrite(8, LOW);
   digitalWrite(9, LOW);

   pinMode(3, OUTPUT);
   pinMode(5, OUTPUT);
   pinMode(10, OUTPUT);
   pinMode(11, OUTPUT);

   analogWrite(3, 166);
   analogWrite(5, 166);
   analogWrite(10, 166);
   analogWrite(11, 166);

   Serial.begin(19200);
   Serial.println("CPU_B");
   sTicker = 0; //for animations
   sBeacon = 1; //for serial beacon ID
   sMode = HBREATHE;
   sTickDur = 5;
}

void debugSend(bool bits[])
{
   Serial.print("MASK:");
   for(int i = 0; i < 6; i++)
   {
      if(i%3 == 0) { continue; }
      Serial.print(bits[i]?"1":"0");
   }
   Serial.print("\nVAL:");
   for(int i = 6; i < 18; i++)
   {
      if(i%3 == 0) { continue; }
      Serial.print(bits[i]?"1":"0");
   }
   Serial.print("\n");
}

void writeToBody(int val)
{
   val = (val * 3) / 5;
   val = val % 180; //avoid burning out LED
   analogWrite(3, val);
   analogWrite(5, val);
   analogWrite(10, val);
   analogWrite(11, val);
}
void writeToBar(char mask, int val)
{
   bool bits[18];
   char ind;
   bits[1] = (mask/8)%2;
   bits[2] = (mask/4)%2;
   bits[4] = (mask/2)%2;
   bits[5] = (mask)%2;

   bits[7] = (val/128)%2;
   bits[8] = (val/64)%2;
   bits[10] = (val/32)%2;
   bits[11] = (val/16)%2;
   bits[13] = (val/8)%2;
   bits[14] = (val/4)%2;
   bits[16] = (val/2)%2;
   bits[17] = (val)%2;

   for(int i = 0; i < 6; i++)
   {
      ind = i*3;
      bits[ind] = (bits[ind+2] != bits[ind+1]);
   }

   //send trigger
   delayMicroseconds(20);
   digitalWrite(7, HIGH);
   digitalWrite(8, HIGH);
   digitalWrite(9, HIGH);
   //start sending
   delayMicroseconds(20); //start of mask
   digitalWrite(7, bits[0]);
   digitalWrite(8, bits[1]);
   digitalWrite(9, bits[2]);
   delayMicroseconds(20);
   digitalWrite(7, bits[3]);
   digitalWrite(8, bits[4]);
   digitalWrite(9, bits[5]);
   delayMicroseconds(20); //start of val
   digitalWrite(7, bits[6]);
   digitalWrite(8, bits[7]);
   digitalWrite(9, bits[8]);
   delayMicroseconds(20);
   digitalWrite(7, bits[9]);
   digitalWrite(8, bits[10]);
   digitalWrite(9, bits[11]);
   delayMicroseconds(20);
   digitalWrite(7, bits[12]);
   digitalWrite(8, bits[13]);
   digitalWrite(9, bits[14]);
   delayMicroseconds(20);
   digitalWrite(7, bits[15]);
   digitalWrite(8, bits[16]);
   digitalWrite(9, bits[17]);
   delayMicroseconds(20);
   //drop to low (idle mode)
   digitalWrite(7, LOW);
   digitalWrite(8, LOW);
   digitalWrite(9, LOW);

   //if(sTicker%8 == 0)debugSend(bits);
}

void pulseMode()
{
   int val;
  
   if(sTicker > 255)
   {
      val = 255 - (sTicker%256);
   }
   else
   {
      val = sTicker;
   }
   writeToBar(0x0f, val);
   writeToBody(val);
}

long expVal(long in)
{
   long val;
   
   if(in > 255)
   {
      val = 255 - (in%256);
   }
   else
   {
      val = in;
   }

   val = val * val / 255;

   return val;
}

void pulseExpMode()
{
   long val = expVal(sTicker);
   writeToBar(0x0f, val);
   writeToBody(val);
}

void breathe()
{
   long val = expVal(sTicker);
   long valD = expVal((sTicker+40)%512);
   long valDD = expVal((sTicker+60)%512);
   
   writeToBar(0x09, val);
   writeToBar(0x06, valD);
   writeToBody(valDD);
}

void hBreathe()
{
   long val = expVal(sTicker);
   long valD = expVal((sTicker+40)%512);
   long valDD = expVal((sTicker+60)%512);
   
   writeToBar(0x09, val);
   writeToBar(0x06, valD);
   writeToBody((valDD)/2);
}

void loop() 
{
  sTicker = (sTicker + 1)%512;
  if(sTicker == 0) { sBeacon = (sBeacon + 1)%100; }
  if(sBeacon == 0)
  {
     Serial.println("CPU_B");
     sBeacon++;
  }
  
  switch(sMode)
  {
     case PULSEM: pulseMode();
        break;
     case PULSEXPM: pulseExpMode();
        break;
     case BREATHE: breathe();
        break;
     case HBREATHE: hBreathe();
        break;
     default:
        writeToBar(0x7f, 255);
  }
  
  delay(sTickDur);
}

