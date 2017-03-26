//code for battery powered animated power button

//LED pins
#define O_PWR_LD 3
#define O_SPWR_LD 5
#define O_HD_LD 6
//analog input pins
#define A_PWR_P 2
#define A_PWR_N 3
#define A_HD_P 6
#define A_HD_N 7
//external sleep signal
#define S_SLP 2

enum PStatus { SLEEP, OFF, ON };

PStatus gStatus;
int gOffTick;
int gOnTick;
int gFadeTick;
int gTick;

void setup()
{                
  pinMode(O_PWR_LD, OUTPUT);
  pinMode(O_SPWR_LD, OUTPUT);
  pinMode(O_HD_LD, OUTPUT);
  pinMode(S_SLP, INPUT);//has external pull down resistor
  
  gStatus = OFF;
  gOffTick = 1023; //off tick ceiling
  gOnTick = 0; //assume off
  gTick = 0;
  gFadeTick = 0;
  
  analogWrite(O_PWR_LD,250);
  analogWrite(O_SPWR_LD,150);
}

int fadeValue()
{
  return gFadeTick > 100 ? 100 - ((gFadeTick%100) * (gFadeTick%100) / 100)
                         : gFadeTick * gFadeTick / 100;
}

void showOn()
{
  analogWrite(O_PWR_LD,250);
  analogWrite(O_SPWR_LD,150);
}
void showOff()
{
  analogWrite(O_PWR_LD,fadeValue());
  analogWrite(O_SPWR_LD,0);
}
void showSleep()
{
  analogWrite(O_PWR_LD,fadeValue()+1);
  analogWrite(O_SPWR_LD,fadeValue()+1);  
}

void updateHD()
{
  int v = abs(analogRead(A_HD_P)-analogRead(A_HD_N));
  digitalWrite(O_HD_LD, v > 200 ? HIGH : LOW); //greater than ~1v
}

void loop()
{
  updateHD();
  bool forceSleep = digitalRead(S_SLP);
  int v = abs(analogRead(A_PWR_P)-analogRead(A_PWR_N));
  
  gTick = (gTick + 1)%15;
  
  if(gTick == 0) { gFadeTick = (gFadeTick + 1)%200; }
  
  if(v > 200)
  {
    gOffTick = 0;
    gOnTick = gOnTick < 1023 ? gOnTick+1 : gOnTick;
  }
  else
  {
    gOnTick = 0;
    gOffTick = gOffTick < 1023 ? gOffTick+1 : gOffTick;
  }
  
  if(forceSleep)
  {
    gStatus = SLEEP;
  }
  else
  {
    switch(gStatus)
    {
      case ON:
        if(gOffTick > 5) { gStatus = SLEEP; }
        break;
      case SLEEP:
        if(gOnTick > 500) { gStatus = ON; }
        else if(gOffTick > 1000) { gStatus = OFF; }
        break;
      case OFF:
      default:
        if(gOnTick > 0) { gStatus = ON; }
    }
  }
  
  switch(gStatus)
  {
    case ON:
      showOn();
      break;
    case SLEEP:
      showSleep();
      break;
    case OFF:
    default:
      showOff();
  }
  
  delay(1);
}

