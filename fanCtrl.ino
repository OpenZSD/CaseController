/* Author: Jerry Z.
 * Code for Tau (arduino compat.) board for controlling LEDs and fans.
 */

#include <FlashStorage.h>
#include <FlashAsEEPROM.h>

#include <math.h>
#include <stdio.h>

#define V_PI 3.1415
#define V_OFFSET 0.3926

//pins
#define OUT_FAN 10
#define IN_FAN 11
#define BAY_FAN 12
#define RES_LED 9 //Primary LED strip
#define MB_LED 8 //Secondary LED strip

//pwm levels
#define F_H 255
#define F_M 210
#define F_L 170

//led pwm levels
#define PWM_TICK 2

//eeprom modes
#define FMODE_H 'H'
#define FMODE_M 'M'
#define FMODE_L 'L'

//eeprom index
#define E_FIN 0
#define E_FOUT 1
#define E_FBAY 2
#define E_L_DIV 3
#define E_L_MBOFF 4
#define E_L_MODE 5

enum LEDMode
{
  SOLID = 'S',
  PULSE = 'P',
  OFF = 'O'
};

void setup()
{
  Serial.begin(9600);
  
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);

  if(!EEPROM.isValid())
  {
    EEPROM.write(E_FIN, 'H');
    EEPROM.write(E_FOUT, 'H');
    EEPROM.write(E_FBAY, 'H');
    EEPROM.write(E_L_MODE, 'P');
    EEPROM.write(E_L_DIV, 4);
    EEPROM.write(E_L_MBOFF, 2);
    EEPROM.commit();
  }

  updateFans();
}

/* Main loop primarily addresses LED pulses with user input checks
*/
void loop()
{
  int sysTick = 0;
  int idTick = 0;
  int pulseTick = 0;
  int pulseCycle;
  float phase;
  float mbPhase;
  float phaseShift;

  auto flr = [](int val)->int
  {
    return val < 5 ? 5 : val;
  };
  
  updatePulseRate(pulseCycle, phaseShift);
  LEDMode mode = getLEDMode();

  delay(1000);
  sendID();
  
  while(1)
  {
    sysTick = (sysTick + 1) % 10; //cycles every 1/100s
    if(!sysTick)
    {
      idTick = (idTick + 1) % 360000; //1hr timer
      if(!idTick) { sendID(); }

      if(mode == PULSE)
      {
        pulseTick = (pulseTick + 1) % pulseCycle;
        phase =  (V_PI * pulseTick) / (float) pulseCycle;
        mbPhase = phase + phaseShift;
        mbPhase = mbPhase > V_PI ? mbPhase - V_PI : mbPhase;
  
        analogWrite(RES_LED, flr(235 * sin(phase) * sin(phase))+15);
        analogWrite(MB_LED, flr(235 * sin(mbPhase) * sin(mbPhase))+15);
      }
      else
      {
        if(mode == SOLID)
        {
          analogWrite(RES_LED, 255);
          analogWrite(MB_LED, 255);
        }
        else
        {
          analogWrite(RES_LED, 0);
          analogWrite(MB_LED, 0);
        }
      }
    }

    if(Serial.available()) { checkUserInput(pulseCycle, phaseShift, mode); }
    delay(1);
  }
}

void checkUserInput(int &pulseCycle, float &phaseShift, LEDMode &mode)
{
  char input = Serial.read();
  char args[3];
  
  while(input != '>' && Serial.available()) { input = Serial.read(); }
  if(!Serial.available())
  {
    sendHelp(); 
    return;
  }

  input = Serial.read();
  switch(input)
  {
    case 'F':
      if(Serial.available() < 3) { return; }
      Serial.readBytes(args, 3);
      setFans(args[0], args[1],args[2]);
      updateFans();
      break;
    case 'R':
      if(!Serial.available()) { return; }
      setPRPwr(Serial.read()-'0');
      updatePulseRate(pulseCycle, phaseShift);
      break;
    case 'O':
      if(!Serial.available()) { return; }
      setMbOffset(Serial.read()-'0');
      updatePulseRate(pulseCycle, phaseShift);
      break;
    case 'L':
      if(!Serial.available()) { return; }
      setLEDMode(mode = charToEnum(Serial.read()));
      break;
    case 'S': EEPROM.commit();
      break;
    case 'D': sendDebug();
      break;
    case 'I': sendID();
      break;
    default: sendHelp();
  }
}

void updatePulseRate(int &pulseCycle, float &phaseShift)
{
  pulseCycle = 12800 / toDiv(getPRPwr()); // 1s cycle at max rate
  phaseShift = ((float) getMbOffset()) * V_OFFSET;
}

LEDMode charToEnum(char c)
{
  switch(c)
  {
    case PULSE: return PULSE;
    case SOLID: return SOLID;
    default: return OFF;
  }
}

void sendID() { Serial.print("PWM_CTRL\n"); }

void sendHelp()
{
  Serial.println("--Command Guide--");
  Serial.println("Input format:\"-->COMMAND\"");
  Serial.println("Command format: <command character><argument character(s)>");
  Serial.println("Example: \"-->LO\" sets (L)ED mode to (O)ff where L is the command and O is the argument");
  Serial.println("-Commands-");
  Serial.println("(F)an mode: F<in-take, exhaust, drive bay> [(H)igh,(M)edium,(L)ow]");
  Serial.println("Example, set all high: \"-->FHHH\"");
  Serial.println("");
  Serial.println("(L)ED mode: L<mode> [(S)olid,(P)ulse,(O)ff]");
  Serial.println("");
  Serial.println("LED pulse (R)ate: P<devisor as power of 2> [0-7]");
  Serial.println("Pulse period = 128s / divisor  [EX: \"-->R3\" -> 128s/8 = 16s]");
  Serial.println("");
  Serial.println("Secondary (O)ffset: O<eighths of cycle> [0-7]");
  Serial.println("[EX: \"-->O4\" -> second set starts half way into first set pulse cycle]");
  Serial.println("");
  Serial.println("(S)ave: S");
  Serial.println("Save settings");
  Serial.println("");
  Serial.println("(D)ebug: D");
  Serial.println("Shows values");
  Serial.println("(I)D: I");
  Serial.println("Sends ID tag");
  Serial.println("");
}

void sendDebug()
{
  char line[256];
  sendID();
  sprintf(line, "FANS IN:%c OUT:%c BAY:%c", EEPROM.read(E_FIN), EEPROM.read(E_FOUT), EEPROM.read(E_FBAY));
  Serial.println(line);
  sprintf(line, "LED MODE:%c RATE:%fs OFFSET:%d/8", EEPROM.read(E_L_MODE), 128.0/(float)toDiv(EEPROM.read(E_L_DIV)), (int)EEPROM.read(E_L_MBOFF));
  Serial.println(line);
  
}

int modeToPwm(char mode)
{
  switch(mode)
  {
    case FMODE_L: return F_L;
    case FMODE_M: return F_M;
    default: return F_H; //default value is high
  }
}

int filtByte(unsigned int pwrOfTwo) { return pwrOfTwo % 8; } //filters to first 3 bits (0 to 7)
int toDiv(unsigned int pwrOfTwo) { return pow(2, filtByte(pwrOfTwo)); }

LEDMode getLEDMode() { return charToEnum(EEPROM.read(E_L_MODE)); }
void setLEDMode(LEDMode mode) { EEPROM.write(E_L_MODE, mode); }

int getPRPwr() { return filtByte(EEPROM.read(E_L_DIV)); }
void setPRPwr(unsigned int pwrOfTwo) { EEPROM.write(E_L_DIV, filtByte(pwrOfTwo)); }

int getMbOffset() { return filtByte(EEPROM.read(E_L_MBOFF)); }
void setMbOffset(unsigned int eighthOfPie) { EEPROM.write(E_L_MBOFF, filtByte(eighthOfPie)); }

void setFans(char in, char out, char bay)
{
  auto sanitize = [](char c)->char { return (c == 'L' || c == 'M') ? c : 'H'; };
  delay(1000);
  EEPROM.write(E_FIN, sanitize(in));
  EEPROM.write(E_FOUT, sanitize(out));
  EEPROM.write(E_FBAY, sanitize(bay));
}
void updateFans()
{
  analogWrite(IN_FAN, modeToPwm(EEPROM.read(E_FIN)));
  analogWrite(OUT_FAN, modeToPwm(EEPROM.read(E_FOUT)));
  analogWrite(BAY_FAN, modeToPwm(EEPROM.read(E_FBAY)));
}

