
void setup()
{
  // put your setup code here, to run once:
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);

  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
}

void loop()
{
  short wave[] = {5,10,20,100,200,250,300,250,200,100,20,10};
  short k = 0;
  
  while(1)
  {
    k = 0;
    
    for(int tick = 0; tick < 100000; tick++)
    {
    
      if(tick%290 == 0) { k++; }
    
      for(int i = 2; i < 13; i++)
      {
        drawWave(wave[(k+i)%12], i, tick);
      }
    }
  }
}

void drawWave(short v, int pin, long tick)
{
  pin = 14-pin;
  if(v < 200)
  {
    if(tick%v == 0) { digitalWrite(pin, HIGH); }
    else { digitalWrite(pin, LOW); }
  }
}

