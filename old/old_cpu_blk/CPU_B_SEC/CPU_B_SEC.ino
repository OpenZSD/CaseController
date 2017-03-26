void setup() 
{
   pinMode(10, INPUT);
   pinMode(11, INPUT);
   pinMode(12, INPUT);

   pinMode(3, OUTPUT);
   pinMode(5, OUTPUT);
   pinMode(6, OUTPUT);
   pinMode(9, OUTPUT);

   analogWrite(3, 255);
   analogWrite(5, 255);
   analogWrite(6, 255);
   analogWrite(9, 255);

   Serial.begin(19200);
   Serial.println("CPU_B_SEC");
}

void loop()
{
   int cmd;
   int mask;
  
   for(int i = 0; i < 10000; i++)
   {
      cmd = pollCmd();
      if(cmd != -1)
      {
         //Serial.println(cmd, BIN);
         mask = cmd / 256;
         if(mask & 0x01) { analogWrite(3, cmd%256); }
         if(mask & 0x02) { analogWrite(5, cmd%256); }
         if(mask & 0x04) { analogWrite(6, cmd%256); }
         if(mask & 0x08) { analogWrite(9, cmd%256); }
      }
      

   }
   Serial.println("CPU_B_SEC");
}

bool validB(bool a, bool b, bool c)
{
   return (b != c) == a; //a is an xor bit
}

int pollCmd()
{
   int val = -1;
   bool a = digitalRead(10);
   bool b = digitalRead(11);
   bool c = digitalRead(12);
   bool bits[12];
   short ind = 0;
   
   bool hit = false;
   bool trig;
   for(int i = 0; i < 80; i++) //80 tries
   {
      delayMicroseconds(5);
      trig = !a && !b && !c;
      a = digitalRead(10);
      b = digitalRead(11);
      c = digitalRead(12);

      //start reading if all 3 are rising edge
      hit = trig && a && b && c;
      if(hit) { break; } //start reading
   }

   if(hit)
   {
      //delay to next bits and a little more
      delayMicroseconds(25);
      val = 0;

      for(int i = 0; i < 6; i++)
      {
         a = digitalRead(10);
         b = digitalRead(11);
         c = digitalRead(12);
        
         if(!validB(a,b,c))
         {
            //do 2 tries to get clean read
            delayMicroseconds(5);
            a = digitalRead(10);
            b = digitalRead(11);
            c = digitalRead(12);
         }
         if(!validB(a,b,c))
         {
            //delay to next cmd and try again
            delayMicroseconds(2420);
            Serial.print("ERROR: Bad read at pair #");
            Serial.println(i);
            return -1;
         }
         
         bits[ind] = b;
         bits[ind+1] = c;
         ind += 2;
         delayMicroseconds(20);
      }

      for(int i = 0; i < 12; i++)
      {
         val *= 2;
         if(bits[i]) { val++; }
      }
   }
   
   return val;
}

