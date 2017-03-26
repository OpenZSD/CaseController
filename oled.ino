/*WARNING, this sketch is known to "lockout" if too large.
 *If changing the animation, take extra caution not to overload
 *board. In the event of a "lockout", compile and upload a simple
 *sketch such as blink. Make sure to press reset upone clicking upload
 *and to compile before hand to avoid the main routine starting before
 *upload begins.*/

/*Changing animations: make sure to factor in the timing so that
 *the animation loop is as close to a minute as possible.
 */

/*Modded by Jerry Z. 2/1/15 */

/* Testing code for driving UG2828GDEAF01 1,5# OLED
 ZhangFeng, PRC.
 zjjszhangf@gmail.com
 http://vfdclock.jimdo.com
 09-12-2013
 */
#include <stdio.h>
#include <math.h>
//#define FASTIO 0

// 4-wire SPI(BS1=0; BS2=0)
#define RST_PIN 4
#define CS_PIN  5
#define DC_PIN 6
#define DIN_PIN 7
#define CLK_PIN 8
#define HVEN_PIN 9

#define DODELAY __asm__("nop\n\t""nop\n\t");
#define COUNTOF(arr) (sizeof(arr) / sizeof(arr[0]))

#ifndef FASTIO
// rst
#define OLED_RES_1 digitalWrite(RST_PIN, HIGH)
#define OLED_RES_0 digitalWrite(RST_PIN, LOW)
// cs
#define OLED_CS_1 digitalWrite(CS_PIN, HIGH)
#define OLED_CS_0 digitalWrite(CS_PIN, LOW)
// DC (Data or Command)
#define OLED_DC_1 digitalWrite(DC_PIN, HIGH)
#define OLED_DC_0 digitalWrite(DC_PIN, LOW)
// data in
#define OLED_DIN_1 digitalWrite(DIN_PIN, HIGH)
#define OLED_DIN_0 digitalWrite(DIN_PIN, LOW)
// clock
#define OLED_CLK_1 digitalWrite(CLK_PIN, HIGH)
#define OLED_CLK_0 digitalWrite(CLK_PIN, LOW)

// HVEN(optional)
#define OLED_HVEN_1 digitalWrite(HVEN_PIN, HIGH)
#define OLED_HVEN_0 digitalWrite(HVEN_PIN, LOW)
#else
// Fast IO version
// pin map please check: http://arduino.cc/en/Hacking/Atmega168Hardware
// pin11 = pb3; pin12 = pb4; pin15 = pb5
#define GPIO_Pin_0                 (0x01)  /* Pin 0 selected */
#define GPIO_Pin_1                 (0x02)  /* Pin 1 selected */
#define GPIO_Pin_2                 (0x04)  /* Pin 2 selected */
#define GPIO_Pin_3                 (0x08)  /* Pin 3 selected */
#define GPIO_Pin_4                 (0x10)  /* Pin 4 selected */
#define GPIO_Pin_5                 (0x20)  /* Pin 5 selected */
#define GPIO_Pin_6                 (0x40)  /* Pin 6 selected */
#define GPIO_Pin_7                 (0x80)  /* Pin 7 selected */

#define RES_PORT PORTB
#define CS_PORT PORTB
#define DC_PORT PORTB
#define DIN_PORT PORTB
#define CLK_PORT PORTB
#define HVEN_PORT PORTB
#define RES_PORTPIN GPIO_Pin_0
#define CS_PORTPIN GPIO_Pin_1
#define DC_PORTPIN GPIO_Pin_2
#define DIN_PORTPIN GPIO_Pin_3
#define CLK_PORTPIN GPIO_Pin_4
#define HVEN_PORTPIN GPIO_Pin_5

#define OLED_RES_1 RES_PORT |= (RES_PORTPIN)
#define OLED_RES_0 RES_PORT &= ~(RES_PORTPIN)
#define OLED_CS_1 CS_PORT |= (CS_PORTPIN)
#define OLED_CS_0 CS_PORT &= ~(CS_PORTPIN)
#define OLED_DC_1 DC_PORT |= (DC_PORTPIN)
#define OLED_DC_0 DC_PORT &= ~(DC_PORTPIN)
#define OLED_DIN_1 DIN_PORT |= (DIN_PORTPIN)
#define OLED_DIN_0 DIN_PORT &= ~(DIN_PORTPIN)
#define OLED_CLK_1 CLK_PORT |= (CLK_PORTPIN)
#define OLED_CLK_0 CLK_PORT &= ~(CLK_PORTPIN)
#define OLED_HVEN_1 HVEN_PORT |= (HVEN_PORTPIN)
#define OLED_HVEN_0 HVEN_PORT &= ~(HVEN_PORTPIN)
#endif

#define Max_Column	0x7f			// 128-1
#define Max_Row		0x7f			// 128-1
#define	Brightness	0x0F

#define MAKECOLOR64k(r,g,b) (unsigned short int)((((unsigned short int)(r)>>3)<<11)|(((unsigned short int)(g)>>2)<<5)|((unsigned short int)(b)>>3))
                               
unsigned short int convTo64k(unsigned char c)
{
  unsigned short int color64k;
  
  if(c == 0x00)
  {
    color64k = 0x0000; //pure black
  }
  else if(c == 0xff)
  {
    color64k = 0xffff; //pure white
  }
  else
  {
    color64k = (c & 0x03) << 3; //B
    color64k |= (c & 0x1c) << 6; //G
    color64k |= (c & 0xe0) << 8; //R
    color64k |= 0x03; //blue bias
    color64k |= 0x80; //green bias
    color64k |= 0x2000; //red bias
  }
  
  return color64k;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Delay Time
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void uDelay(unsigned char l)
{
  while(l--)
  {
    //__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    //__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");
  }; 
}

void Write_Command(unsigned char Data)
{
  unsigned char i;
  OLED_CS_0;
  OLED_DC_0;
  for (i=0; i<8; i++)
  {
    OLED_CLK_0;
    //((Data&0x80)>>7)?OLED_DIN_1:OLED_DIN_0;
    if((Data&0x80)>>7)
      OLED_DIN_1;
    else 
      OLED_DIN_0;
    Data = Data << 1;
    uDelay(1);
    OLED_CLK_1;
    uDelay(1);
  }
  OLED_CLK_0;
  OLED_DC_1;
  OLED_CS_1;
}

void Write_Data(unsigned char Data)
{
  unsigned char i;

  OLED_CS_0;
  OLED_DC_1;
  for (i=0; i<8; i++)
  {
    OLED_CLK_0;
    if((Data&0x80)>>7)
      OLED_DIN_1;
    else 
      OLED_DIN_0;
    Data = Data << 1;
    uDelay(1);
    OLED_CLK_1;
    uDelay(1);
  }
  OLED_CLK_0;
  OLED_DC_1;
  OLED_CS_1;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Gray Scale Table Setting (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Set_Gray_Scale_Table(void)
{
  Write_Command(0xB8);
  Write_Data(0x01);			// Gray Scale Level 1
  Write_Data(0x03);			// Gray Scale Level 3
  Write_Data(0x05);			// Gray Scale Level 5
  Write_Data(0x07);			// Gray Scale Level 7
  Write_Data(0x0B);			// Gray Scale Level 9
  Write_Data(0x10);			// Gray Scale Level 11
  Write_Data(0x16);			// Gray Scale Level 13
  Write_Data(0x1A);			// Gray Scale Level 15
  Write_Data(0x1F);			// Gray Scale Level 17
  Write_Data(0x24);			// Gray Scale Level 19
  Write_Data(0x29);			// Gray Scale Level 21
  Write_Data(0x2E);			// Gray Scale Level 23
  Write_Data(0x33);			// Gray Scale Level 25
  Write_Data(0x38);			// Gray Scale Level 27
  Write_Data(0x3D);			// Gray Scale Level 29
  Write_Data(0x42);			// Gray Scale Level 31
  Write_Data(0x47);			// Gray Scale Level 33
  Write_Data(0x4C);			// Gray Scale Level 35
  Write_Data(0x51);			// Gray Scale Level 37
  Write_Data(0x56);			// Gray Scale Level 39
  Write_Data(0x5A);			// Gray Scale Level 41
  Write_Data(0x5E);			// Gray Scale Level 43
  Write_Data(0x62);			// Gray Scale Level 45
  Write_Data(0x65);			// Gray Scale Level 47
  Write_Data(0x68);			// Gray Scale Level 49
  Write_Data(0x6B);			// Gray Scale Level 51
  Write_Data(0x6E);			// Gray Scale Level 53
  Write_Data(0x71);			// Gray Scale Level 55
  Write_Data(0x74);			// Gray Scale Level 57
  Write_Data(0x77);			// Gray Scale Level 59
  Write_Data(0x7A);			// Gray Scale Level 61
  Write_Data(0x7D);			// Gray Scale Level 63
}

void Clear_Window(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
  Write_Command(0x8E);
  Write_Data(a);				// Column Address of Start
  Write_Data(c);				// Row Address of Start
  Write_Data(b);				// Column Address of End
  Write_Data(d);				// Row Address of End
  delay(10); // delay 5ms for its drawing 
}


void OLED_InitIOs(void)
{
  OLED_RES_1;
  OLED_CS_1;
  OLED_CLK_1;
  OLED_DIN_1; 
  OLED_DC_1;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Initialization
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void OLED_Init(void)
{
  unsigned char i;

  OLED_RES_0;
  for(i=0;i<200;i++)
  {
    delay(1); // delay
  }
  OLED_RES_1;

  // Display Off (0x00/0x01)
  Write_Command(0xAE);
  Write_Command(0xA0);
  Write_Data(0x74); // [0x74]: 64k Colors Mode; [0xB4]:262k Colors Mode;256 Colors Mode (0x34)
  Write_Command(0xA1);
  Write_Data(0x00);
  Write_Command(0xA2);
  Write_Data(0x80);
  Write_Command(0xA6);
  Write_Command(0xAD);
  Write_Data(0x8E);
  Write_Command(0xB0);
  Write_Data(0x05);
  Write_Command(0xB1);
  Write_Data(0x11);
  Write_Command(0xB3);
  Write_Data(0xF0);
  // gray table
  Set_Gray_Scale_Table();

  Write_Command(0xBB);
  Write_Data(0x1C);
  Write_Data(0x1C);
  Write_Data(0x1C);

  Write_Command(0xBE);
  Write_Data(0x3F);

  Write_Command(0xC1);
  Write_Data(0xDC);
  Write_Data(0xD2);
  Write_Data(0xFF);

  Write_Command(0xC7);
  Write_Data(0x0A);

  Write_Command(0xCA);
  Write_Data(0x7F);

  Write_Command(0xAF);
}

void EnableFill(unsigned char d)
{
  Write_Command(0x92);
  Write_Data(!!d);				// Default => 0x00
}

//===============================================================================//
//  Graphic Acceleration(by hardware)
//===============================================================================//
void Draw_Line_Char(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char col)///////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
  Draw_Line64k(x1,y1,x2,y2, convTo64k(col));
}

void Draw_Line64k(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned short int col)
{
  Write_Command(0x83);
  Write_Data(x1);				// Column Address of Start
  Write_Data(y1);				// Row Address of Start
  Write_Data(x2);				// Column Address of End
  Write_Data(y2);				// Row Address of End
  Write_Data(col>>8);				// Line Color - CCCCCBBB
  Write_Data(col&0xff);			// Line Color - BBBAAAAA
  delay(1); // delay 5ms for its drawing  
}

void Draw_Rectangle64k(unsigned char xLeft, unsigned char yTop, unsigned char xRight, unsigned char yBottom, unsigned short int colOutline, unsigned short int colFill)
{
  Write_Command(0x84);
  Write_Data(xLeft);
  Write_Data(yTop);
  Write_Data(xRight);
  Write_Data(yBottom);
  Write_Data(colOutline>>8);				// Line Color - CCCCCBBB
  Write_Data(colOutline&0xff);			// Line Color - BBBAAAAA
  Write_Data(colFill>>8);				// Line Color - CCCCCBBB
  Write_Data(colFill&0xff);			// Line Color - BBBAAAAA
  delay(2); // delay 5ms for its drawing  
}

void Draw_Circle64k(unsigned char x, unsigned char y, unsigned char r, unsigned short int colOutline, unsigned short int colFill)
{
  Write_Command(0x86);
  Write_Data(x);				// Column Address of Start
  Write_Data(y);				// Row Address of Start
  Write_Data(r);				// Radius
  Write_Data(colOutline>>8);				// Line Color - CCCCCBBB
  Write_Data(colOutline&0xff);			// Line Color - BBBAAAAA
  Write_Data(colFill>>8);				// Fill Color - CCCCCBBB
  Write_Data(colFill&0xff);			// Fill Color - BBBAAAAA
  delayMicroseconds(400); // delay 10ms for its drawing  
}

void Draw_Circle(unsigned char x, unsigned char y, unsigned char r, unsigned char colOutline, unsigned char colFill)
{
  Write_Command(0x86);
  Write_Data(x);				// Column Address of Start
  Write_Data(y);				// Row Address of Start
  Write_Data(r);				// Radius
  Write_Data(colOutline);				// Line Color - CCCCCBBB
  Write_Data(colOutline);			// Line Color - BBBAAAAA
  Write_Data(colFill);				// Fill Color - CCCCCBBB
  Write_Data(colFill);			// Fill Color - BBBAAAAA
  delayMicroseconds(400); // delay 10ms for its drawing  
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Regular Pattern (Partial or Full Screen)
//
//    a: Column Address of Start
//    b: Column Address of End
//    c: Row Address of Start
//    d: Row Address of End
//    e: BBBBBGGG
//    f: GGGRRRRR
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fill_Block64k(unsigned char xLeft, unsigned char yTop, unsigned char xRight, unsigned char yBottom, unsigned short int col)
{
  EnableFill(0x01);
  Draw_Rectangle64k(xLeft, yTop, xRight, yBottom, col, col);
  EnableFill(0x00);
}

void Fill_Block64k_Slow(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned short int col)
{
  unsigned char i,j;

  Set_Column_Address(x1,x2);
  Set_Row_Address(y1,y2);	
  Set_Write_RAM();

  for(i=0;i<(y2-y1+1);i++)
  {
    for(j=0;j<(x2-x1+1);j++)
    {
      Write_Data(col>>8);				// Line Color - CCCCCBBB
      Write_Data(col&0xff);			// Line Color - BBBAAAAA
    }
  }
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Instruction Setting
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Set_Column_Address(unsigned char a, unsigned char b)
{
  Write_Command(0x15);
  Write_Data(a);				// Default => 0x00
  Write_Data(b);				// Default => 0x83
}

void Set_Row_Address(unsigned char a, unsigned char b)
{
  Write_Command(0x75);
  Write_Data(a);				// Default => 0x00
  Write_Data(b);				// Default => 0x83
}

void Set_Write_RAM(void)
{
  Write_Command(0x5C);			// Enable MCU to Write into RAM
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Patterns
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Show_64k_Pattern_BigEndian(unsigned char *Data_Pointer, unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2)
{
  unsigned char *Src_Pointer;
  unsigned char i,j;

  Src_Pointer=Data_Pointer;
  Set_Column_Address(x1,x2);
  Set_Row_Address(y1,y2);
  Set_Write_RAM();

  for(i=0;i<(y2-y1+1);i++)
  {
    for(j=0;j<(x2-x1+1);j++)
    {
      Write_Data(*Src_Pointer);
      Src_Pointer++;
      Write_Data(*Src_Pointer);
      Src_Pointer++;
    }
  }
}

void Show_64k_Pattern_LittleEndia(unsigned char *Data_Pointer, unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2)
{
  unsigned char *Src_Pointer;
  unsigned char i,j;

  Src_Pointer=Data_Pointer;
  Set_Column_Address(x1,x2);
  Set_Row_Address(y1,y2);	
  Set_Write_RAM();

  for(i=0;i<(y2-y1+1);i++)
  {
    for(j=0;j<(x2-x1+1);j++)
    {
      Write_Data(*(Src_Pointer+1));
      Write_Data(*(Src_Pointer));
      Src_Pointer+=2;
    }
  }
}

void Show_Char_Pattern(unsigned char *Data_Pointer, unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2)
{
  unsigned char *Src_Pointer;
  unsigned char i,j;

  /*Src_Pointer=Data_Pointer;
  Set_Column_Address(x1,x2);
  Set_Row_Address(y1,y2);	
  Set_Write_RAM();*/
  
  
  Src_Pointer=Data_Pointer;
  for(i=0;i<(y2-y1+1);i++)
  {
    for(j=0;j<(x2-x1+1);j++)
    {
      //Draw_Line_Char(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, uunsigned char col)
      unsigned char x = x1+j;
      unsigned char y = y1+i;
      
      unsigned char xe = x;
      
      while((j < (x2-x1)) && (Src_Pointer[0] == Src_Pointer[1]))
      {
        xe++;
        Src_Pointer++;
        j++;
      }
      
      if(*(Src_Pointer) != 0x00) { Draw_Line_Char(x,y,xe,y,*(Src_Pointer)); }
      
      //Write_Data(*(Src_Pointer));
      Src_Pointer++;
    }
  }
}

void Show_256k_Pattern(unsigned char *Data_Pointer, unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
  unsigned char *Src_Pointer;
  unsigned char i,j;

  Src_Pointer=Data_Pointer;
  Set_Column_Address(a,b);
  Set_Row_Address(c,d);	
  Set_Write_RAM();

  for(i=0;i<(d-c+1);i++)
  {
    for(j=0;j<(b-a+1);j++)
    {
      Write_Data(*Src_Pointer);
      Src_Pointer++;
      Write_Data(*Src_Pointer);
      Src_Pointer++;
      Write_Data(*Src_Pointer);
      Src_Pointer++;
    }
  }
}

void printTek(int tick)
{
  char xOff = 0;
  float ps = 0.349055556;
  unsigned short phase = 0;
  
  for(int ind = 0; ind < 53; ind++) 
  {
    for(int i = 0; i < 2; i++)
    {
      phase = (ind*2);
      xOff = 0;
      if((tick > 5) && (tick <= 200))
      {
        delay(100);
        return;
      }
      else if(tick > 200)
      {
        if(tick < 346)
        {
          phase += 217;
          if((phase >= tick)&&(phase < (tick+18))) //distortions are 18px tall
          {
            phase -= tick;
            xOff = 16 * sin(phase*ps);
          }
          else
          {
            continue;
          }
        }
        else if((tick < 546) && (tick > 400))
        {
          phase += 417;
          if((phase >= tick)&&(phase < (tick+18))) //distortions are 18px tall
          {
            phase -= tick;
            xOff = 16 * sin(phase*ps);
          }
          else
          {
            continue;
          }
        }
        else
        {
          delay(100);
          return;
        }
      }
      
      switch(ind)
      {
        case 0: Draw_Line_Char(0,13+i,127,13+i,0x00);
                Draw_Line_Char(xOff+108,13+i,xOff+108,13+i,0xff); break;
    
        case 1: Draw_Line_Char(0,15+i,127,15+i,0x00);
                Draw_Line_Char(xOff+107,15+i,xOff+107,15+i,0xff); break;
    
        case 2: Draw_Line_Char(0,18+i,127,18+i,0x00);
                Draw_Line_Char(xOff+105,18+i,xOff+106,18+i,0xff); break;
    
        case 3: Draw_Line_Char(0,20+i,127,20+i,0x00);
                Draw_Line_Char(xOff+77,20+i,xOff+81,20+i,0xff);
                Draw_Line_Char(xOff+85,20+i,xOff+88,20+i,0xff);
                Draw_Line_Char(xOff+93,20+i,xOff+101,20+i,0xff); break;
    
        case 4: Draw_Line_Char(0,22+i,127,22+i,0x00);
                Draw_Line_Char(xOff+81,22+i,xOff+95,22+i,0xff); break;
    
        case 5: Draw_Line_Char(0,24+i,127,24+i,0x00);
                Draw_Line_Char(xOff+70,24+i,xOff+94,24+i,0xff); break;
    
        case 6: Draw_Line_Char(0,26+i,127,26+i,0x00);
                Draw_Line_Char(xOff+65,26+i,xOff+65,26+i,0xff);
                Draw_Line_Char(xOff+69,26+i,xOff+69,26+i,0xff);
                Draw_Line_Char(xOff+72,26+i,xOff+76,26+i,0xff);
                Draw_Line_Char(xOff+79,26+i,xOff+84,26+i,0xff); break;
    
        case 7: Draw_Line_Char(0,28+i,127,28+i,0x00);
                Draw_Line_Char(xOff+67,28+i,xOff+68,28+i,0xff);
                Draw_Line_Char(xOff+72,28+i,xOff+74,28+i,0xff);
                Draw_Line_Char(xOff+76,28+i,xOff+81,28+i,0xff); break;
    
        case 8: Draw_Line_Char(0,30+i,127,30+i,0x00);
                Draw_Line_Char(xOff+63,30+i,xOff+68,30+i,0xff);
                Draw_Line_Char(xOff+72,30+i,xOff+73,30+i,0xff);
                Draw_Line_Char(xOff+75,30+i,xOff+80,30+i,0xff); break;
    
        case 9: Draw_Line_Char(0,32+i,127,32+i,0x00);
                Draw_Line_Char(xOff+62,32+i,xOff+69,32+i,0xff);
                Draw_Line_Char(xOff+71,32+i,xOff+72,32+i,0xff);
                Draw_Line_Char(xOff+75,32+i,xOff+78,32+i,0xff);
                Draw_Line_Char(xOff+80,32+i,xOff+80,32+i,0xff); break;
    
        case 10: Draw_Line_Char(0,34+i,127,34+i,0x00);
                Draw_Line_Char(xOff+62,34+i,xOff+66,34+i,0xff);
                Draw_Line_Char(xOff+68,34+i,xOff+68,34+i,0xff);
                Draw_Line_Char(xOff+71,34+i,xOff+71,34+i,0xff);
                Draw_Line_Char(xOff+75,34+i,xOff+77,34+i,0xff);
                Draw_Line_Char(xOff+81,34+i,xOff+82,34+i,0xff); break;
    
        case 11: Draw_Line_Char(0,36+i,127,36+i,0x00);
                Draw_Line_Char(xOff+62,36+i,xOff+63,36+i,0xff);
                Draw_Line_Char(xOff+65,36+i,xOff+65,36+i,0xff);
                Draw_Line_Char(xOff+68,36+i,xOff+69,36+i,0xff);
                Draw_Line_Char(xOff+74,36+i,xOff+78,36+i,0xff);
                Draw_Line_Char(xOff+82,36+i,xOff+83,36+i,0xff); break;
    
        case 12: Draw_Line_Char(0,38+i,127,38+i,0x00);
                Draw_Line_Char(xOff+63,38+i,xOff+63,38+i,0xff);
                Draw_Line_Char(xOff+65,38+i,xOff+66,38+i,0xff);
                Draw_Line_Char(xOff+68,38+i,xOff+71,38+i,0xff);
                Draw_Line_Char(xOff+74,38+i,xOff+75,38+i,0xff);
                Draw_Line_Char(xOff+77,38+i,xOff+79,38+i,0xff);
                Draw_Line_Char(xOff+82,38+i,xOff+84,38+i,0xff); break;
    
        case 13: Draw_Line_Char(0,40+i,127,40+i,0x00);
                Draw_Line_Char(xOff+63,40+i,xOff+64,40+i,0xff);
                Draw_Line_Char(xOff+69,40+i,xOff+71,40+i,0xff);
                Draw_Line_Char(xOff+75,40+i,xOff+75,40+i,0xff);
                Draw_Line_Char(xOff+78,40+i,xOff+79,40+i,0xff);
                Draw_Line_Char(xOff+82,40+i,xOff+84,40+i,0xff); break;
    
        case 14: Draw_Line_Char(0,42+i,127,42+i,0x00);
                Draw_Line_Char(xOff+62,42+i,xOff+62,42+i,0xff);
                Draw_Line_Char(xOff+66,42+i,xOff+67,42+i,0xff);
                Draw_Line_Char(xOff+70,42+i,xOff+74,42+i,0xff);
                Draw_Line_Char(xOff+79,42+i,xOff+85,42+i,0xff); break;
    
        case 15: Draw_Line_Char(0,44+i,127,44+i,0x00);
                Draw_Line_Char(xOff+67,44+i,xOff+68,44+i,0xff);
                Draw_Line_Char(xOff+74,44+i,xOff+76,44+i,0xff);
                Draw_Line_Char(xOff+82,44+i,xOff+86,44+i,0xff); break;
    
        case 16: Draw_Line_Char(0,46+i,127,46+i,0x00);
                Draw_Line_Char(xOff+68,46+i,xOff+71,46+i,0xff);
                Draw_Line_Char(xOff+75,46+i,xOff+77,46+i,0xff);
                Draw_Line_Char(xOff+85,46+i,xOff+90,46+i,0xff); break;
    
        case 17: Draw_Line_Char(0,48+i,127,48+i,0x00);
                Draw_Line_Char(xOff+67,48+i,xOff+72,48+i,0xff);
                Draw_Line_Char(xOff+75,48+i,xOff+76,48+i,0xff);
                Draw_Line_Char(xOff+86,48+i,xOff+90,48+i,0xff); break;
    
        case 18: Draw_Line_Char(0,50+i,127,50+i,0x00);
                Draw_Line_Char(xOff+58,50+i,xOff+58,50+i,0xff);
                Draw_Line_Char(xOff+62,50+i,xOff+72,50+i,0xff);
                Draw_Line_Char(xOff+86,50+i,xOff+91,50+i,0xff); break;
    
        case 19: Draw_Line_Char(0,52+i,127,52+i,0x00);
                Draw_Line_Char(xOff+43,52+i,xOff+46,52+i,0xff);
                Draw_Line_Char(xOff+56,52+i,xOff+57,52+i,0xff);
                Draw_Line_Char(xOff+65,52+i,xOff+71,52+i,0xff);
                Draw_Line_Char(xOff+78,52+i,xOff+79,52+i,0xff);
                Draw_Line_Char(xOff+85,52+i,xOff+87,52+i,0xff);
                Draw_Line_Char(xOff+90,52+i,xOff+91,52+i,0xff);
                Draw_Line_Char(xOff+96,52+i,xOff+96,52+i,0xff); break;
    
        case 20: Draw_Line_Char(0,54+i,127,54+i,0x00);
                Draw_Line_Char(xOff+42,54+i,xOff+43,54+i,0xff);
                Draw_Line_Char(xOff+56,54+i,xOff+57,54+i,0xff);
                Draw_Line_Char(xOff+64,54+i,xOff+65,54+i,0xff);
                Draw_Line_Char(xOff+68,54+i,xOff+69,54+i,0xff);
                Draw_Line_Char(xOff+75,54+i,xOff+77,54+i,0xff);
                Draw_Line_Char(xOff+82,54+i,xOff+83,54+i,0xff);
                Draw_Line_Char(xOff+89,54+i,xOff+90,54+i,0xff);
                Draw_Line_Char(xOff+96,54+i,xOff+96,54+i,0xff); break;
    
        case 21: Draw_Line_Char(0,56+i,127,56+i,0x00);
                Draw_Line_Char(xOff+41,56+i,xOff+42,56+i,0xff);
                Draw_Line_Char(xOff+44,56+i,xOff+48,56+i,0xff);
                Draw_Line_Char(xOff+54,56+i,xOff+68,56+i,0xff);
                Draw_Line_Char(xOff+75,56+i,xOff+77,56+i,0xff);
                Draw_Line_Char(xOff+80,56+i,xOff+82,56+i,0xff);
                Draw_Line_Char(xOff+87,56+i,xOff+90,56+i,0xff);
                Draw_Line_Char(xOff+93,56+i,xOff+95,56+i,0xff); break;
    
        case 22: Draw_Line_Char(0,58+i,127,58+i,0x00);
                Draw_Line_Char(xOff+42,58+i,xOff+44,58+i,0xff);
                Draw_Line_Char(xOff+48,58+i,xOff+48,58+i,0xff);
                Draw_Line_Char(xOff+54,58+i,xOff+55,58+i,0xff);
                Draw_Line_Char(xOff+58,58+i,xOff+63,58+i,0xff);
                Draw_Line_Char(xOff+73,58+i,xOff+75,58+i,0xff);
                Draw_Line_Char(xOff+83,58+i,xOff+87,58+i,0xff);
                Draw_Line_Char(xOff+91,58+i,xOff+92,58+i,0xff); break;
    
        case 23: Draw_Line_Char(0,60+i,127,60+i,0x00);
                Draw_Line_Char(xOff+47,60+i,xOff+50,60+i,0xff);
                Draw_Line_Char(xOff+53,60+i,xOff+54,60+i,0xff);
                Draw_Line_Char(xOff+57,60+i,xOff+60,60+i,0xff);
                Draw_Line_Char(xOff+72,60+i,xOff+72,60+i,0xff);
                Draw_Line_Char(xOff+82,60+i,xOff+88,60+i,0xff);
                Draw_Line_Char(xOff+91,60+i,xOff+91,60+i,0xff); break;
    
        case 24: Draw_Line_Char(0,62+i,127,62+i,0x00);
                Draw_Line_Char(xOff+39,62+i,xOff+39,62+i,0xff);
                Draw_Line_Char(xOff+45,62+i,xOff+50,62+i,0xff);
                Draw_Line_Char(xOff+53,62+i,xOff+53,62+i,0xff);
                Draw_Line_Char(xOff+62,62+i,xOff+63,62+i,0xff);
                Draw_Line_Char(xOff+68,62+i,xOff+68,62+i,0xff);
                Draw_Line_Char(xOff+81,62+i,xOff+90,62+i,0xff); break;
    
        case 25: Draw_Line_Char(0,64+i,127,64+i,0x00);
                Draw_Line_Char(xOff+33,64+i,xOff+36,64+i,0xff);
                Draw_Line_Char(xOff+42,64+i,xOff+47,64+i,0xff);
                Draw_Line_Char(xOff+49,64+i,xOff+52,64+i,0xff);
                Draw_Line_Char(xOff+56,64+i,xOff+56,64+i,0xff);
                Draw_Line_Char(xOff+61,64+i,xOff+61,64+i,0xff);
                Draw_Line_Char(xOff+65,64+i,xOff+67,64+i,0xff);
                Draw_Line_Char(xOff+80,64+i,xOff+86,64+i,0xff); break;
    
        case 26: Draw_Line_Char(0,66+i,127,66+i,0x00);
                Draw_Line_Char(xOff+31,66+i,xOff+32,66+i,0xff);
                Draw_Line_Char(xOff+39,66+i,xOff+51,66+i,0xff);
                Draw_Line_Char(xOff+54,66+i,xOff+54,66+i,0xff);
                Draw_Line_Char(xOff+58,66+i,xOff+58,66+i,0xff);
                Draw_Line_Char(xOff+78,66+i,xOff+82,66+i,0xff); break;
    
        case 27: Draw_Line_Char(0,68+i,127,68+i,0x00);
                Draw_Line_Char(xOff+29,68+i,xOff+38,68+i,0xff);
                Draw_Line_Char(xOff+43,68+i,xOff+47,68+i,0xff);
                Draw_Line_Char(xOff+50,68+i,xOff+52,68+i,0xff);
                Draw_Line_Char(xOff+57,68+i,xOff+57,68+i,0xff);
                Draw_Line_Char(xOff+63,68+i,xOff+84,68+i,0xff); break;
    
        case 28: Draw_Line_Char(0,70+i,127,70+i,0x00);
                Draw_Line_Char(xOff+29,70+i,xOff+35,70+i,0xff);
                Draw_Line_Char(xOff+43,70+i,xOff+46,70+i,0xff);
                Draw_Line_Char(xOff+53,70+i,xOff+56,70+i,0xff);
                Draw_Line_Char(xOff+61,70+i,xOff+62,70+i,0xff);
                Draw_Line_Char(xOff+66,70+i,xOff+71,70+i,0xff); break;
    
        case 29: Draw_Line_Char(0,72+i,127,72+i,0x00);
                Draw_Line_Char(xOff+28,72+i,xOff+32,72+i,0xff);
                Draw_Line_Char(xOff+42,72+i,xOff+44,72+i,0xff);
                Draw_Line_Char(xOff+46,72+i,xOff+47,72+i,0xff);
                Draw_Line_Char(xOff+53,72+i,xOff+53,72+i,0xff);
                Draw_Line_Char(xOff+59,72+i,xOff+61,72+i,0xff);
                Draw_Line_Char(xOff+65,72+i,xOff+68,72+i,0xff); break;
    
        case 30: Draw_Line_Char(0,74+i,127,74+i,0x00);
                Draw_Line_Char(xOff+26,74+i,xOff+29,74+i,0xff);
                Draw_Line_Char(xOff+39,74+i,xOff+41,74+i,0xff);
                Draw_Line_Char(xOff+45,74+i,xOff+48,74+i,0xff);
                Draw_Line_Char(xOff+51,74+i,xOff+58,74+i,0xff);
                Draw_Line_Char(xOff+64,74+i,xOff+68,74+i,0xff);
                Draw_Line_Char(xOff+72,74+i,xOff+72,74+i,0xff); break;
    
        case 31: Draw_Line_Char(0,76+i,127,76+i,0x00);
                Draw_Line_Char(xOff+25,76+i,xOff+27,76+i,0xff);
                Draw_Line_Char(xOff+41,76+i,xOff+41,76+i,0xff);
                Draw_Line_Char(xOff+48,76+i,xOff+59,76+i,0xff);
                Draw_Line_Char(xOff+65,76+i,xOff+69,76+i,0xff);
                Draw_Line_Char(xOff+72,76+i,xOff+72,76+i,0xff); break;
    
        case 32: Draw_Line_Char(0,78+i,127,78+i,0x00);
                Draw_Line_Char(xOff+24,78+i,xOff+25,78+i,0xff);
                Draw_Line_Char(xOff+48,78+i,xOff+57,78+i,0xff);
                Draw_Line_Char(xOff+67,78+i,xOff+69,78+i,0xff);
                Draw_Line_Char(xOff+73,78+i,xOff+73,78+i,0xff); break;
    
        case 33: Draw_Line_Char(0,80+i,127,80+i,0x00);
                Draw_Line_Char(xOff+22,80+i,xOff+24,80+i,0xff);
                Draw_Line_Char(xOff+37,80+i,xOff+37,80+i,0xff);
                Draw_Line_Char(xOff+39,80+i,xOff+40,80+i,0xff);
                Draw_Line_Char(xOff+46,80+i,xOff+60,80+i,0xff);
                Draw_Line_Char(xOff+68,80+i,xOff+69,80+i,0xff);
                Draw_Line_Char(xOff+73,80+i,xOff+74,80+i,0xff); break;
    
        case 34: Draw_Line_Char(0,82+i,127,82+i,0x00);
                Draw_Line_Char(xOff+21,82+i,xOff+23,82+i,0xff);
                Draw_Line_Char(xOff+25,82+i,xOff+25,82+i,0xff);
                Draw_Line_Char(xOff+39,82+i,xOff+39,82+i,0xff);
                Draw_Line_Char(xOff+45,82+i,xOff+58,82+i,0xff);
                Draw_Line_Char(xOff+61,82+i,xOff+61,82+i,0xff);
                Draw_Line_Char(xOff+69,82+i,xOff+71,82+i,0xff);
                Draw_Line_Char(xOff+73,82+i,xOff+73,82+i,0xff); break;
    
        case 35: Draw_Line_Char(0,84+i,127,84+i,0x00);
                Draw_Line_Char(xOff+21,84+i,xOff+22,84+i,0xff);
                Draw_Line_Char(xOff+24,84+i,xOff+24,84+i,0xff);
                Draw_Line_Char(xOff+41,84+i,xOff+59,84+i,0xff);
                Draw_Line_Char(xOff+69,84+i,xOff+72,84+i,0xff); break;
    
        case 36: Draw_Line_Char(0,86+i,127,86+i,0x00);
                Draw_Line_Char(xOff+21,86+i,xOff+21,86+i,0xff);
                Draw_Line_Char(xOff+23,86+i,xOff+24,86+i,0xff);
                Draw_Line_Char(xOff+42,86+i,xOff+61,86+i,0xff);
                Draw_Line_Char(xOff+66,86+i,xOff+66,86+i,0xff);
                Draw_Line_Char(xOff+68,86+i,xOff+71,86+i,0xff); break;
    
        case 37: Draw_Line_Char(0,88+i,127,88+i,0x00);
                Draw_Line_Char(xOff+20,88+i,xOff+20,88+i,0xff);
                Draw_Line_Char(xOff+23,88+i,xOff+24,88+i,0xff);
                Draw_Line_Char(xOff+31,88+i,xOff+32,88+i,0xff);
                Draw_Line_Char(xOff+34,88+i,xOff+35,88+i,0xff);
                Draw_Line_Char(xOff+38,88+i,xOff+59,88+i,0xff);
                Draw_Line_Char(xOff+64,88+i,xOff+71,88+i,0xff); break;
    
        case 38: Draw_Line_Char(0,90+i,127,90+i,0x00);
                Draw_Line_Char(xOff+19,90+i,xOff+19,90+i,0xff);
                Draw_Line_Char(xOff+22,90+i,xOff+25,90+i,0xff);
                Draw_Line_Char(xOff+30,90+i,xOff+36,90+i,0xff);
                Draw_Line_Char(xOff+38,90+i,xOff+61,90+i,0xff);
                Draw_Line_Char(xOff+70,90+i,xOff+71,90+i,0xff); break;
    
        case 39: Draw_Line_Char(0,92+i,127,92+i,0x00);
                Draw_Line_Char(xOff+19,92+i,xOff+19,92+i,0xff);
                Draw_Line_Char(xOff+22,92+i,xOff+25,92+i,0xff);
                Draw_Line_Char(xOff+30,92+i,xOff+61,92+i,0xff);
                Draw_Line_Char(xOff+70,92+i,xOff+71,92+i,0xff); break;
    
        case 40: Draw_Line_Char(0,94+i,127,94+i,0x00);
                Draw_Line_Char(xOff+19,94+i,xOff+19,94+i,0xff);
                Draw_Line_Char(xOff+22,94+i,xOff+27,94+i,0xff);
                Draw_Line_Char(xOff+31,94+i,xOff+31,94+i,0xff);
                Draw_Line_Char(xOff+34,94+i,xOff+37,94+i,0xff);
                Draw_Line_Char(xOff+39,94+i,xOff+61,94+i,0xff);
                Draw_Line_Char(xOff+70,94+i,xOff+70,94+i,0xff); break;
    
        case 41: Draw_Line_Char(0,96+i,127,96+i,0x00);
                Draw_Line_Char(xOff+19,96+i,xOff+19,96+i,0xff);
                Draw_Line_Char(xOff+22,96+i,xOff+30,96+i,0xff);
                Draw_Line_Char(xOff+34,96+i,xOff+40,96+i,0xff);
                Draw_Line_Char(xOff+43,96+i,xOff+60,96+i,0xff);
                Draw_Line_Char(xOff+70,96+i,xOff+70,96+i,0xff); break;
    
        case 42: Draw_Line_Char(0,98+i,127,98+i,0x00);
                Draw_Line_Char(xOff+20,98+i,xOff+20,98+i,0xff);
                Draw_Line_Char(xOff+23,98+i,xOff+32,98+i,0xff);
                Draw_Line_Char(xOff+35,98+i,xOff+61,98+i,0xff);
                Draw_Line_Char(xOff+70,98+i,xOff+70,98+i,0xff); break;
    
        case 43: Draw_Line_Char(0,100+i,127,100+i,0x00);
                Draw_Line_Char(xOff+21,100+i,xOff+21,100+i,0xff);
                Draw_Line_Char(xOff+24,100+i,xOff+34,100+i,0xff);
                Draw_Line_Char(xOff+36,100+i,xOff+38,100+i,0xff);
                Draw_Line_Char(xOff+41,100+i,xOff+62,100+i,0xff);
                Draw_Line_Char(xOff+69,100+i,xOff+69,100+i,0xff); break;
    
        case 44: Draw_Line_Char(0,102+i,127,102+i,0x00);
                Draw_Line_Char(xOff+22,102+i,xOff+22,102+i,0xff);
                Draw_Line_Char(xOff+24,102+i,xOff+36,102+i,0xff);
                Draw_Line_Char(xOff+48,102+i,xOff+66,102+i,0xff);
                Draw_Line_Char(xOff+70,102+i,xOff+70,102+i,0xff); break;
    
        case 45: Draw_Line_Char(0,104+i,127,104+i,0x00);
                Draw_Line_Char(xOff+25,104+i,xOff+36,104+i,0xff);
                Draw_Line_Char(xOff+50,104+i,xOff+66,104+i,0xff);
                Draw_Line_Char(xOff+69,104+i,xOff+69,104+i,0xff); break;
    
        case 46: Draw_Line_Char(0,106+i,127,106+i,0x00);
                Draw_Line_Char(xOff+24,106+i,xOff+24,106+i,0xff);
                Draw_Line_Char(xOff+26,106+i,xOff+35,106+i,0xff);
                Draw_Line_Char(xOff+53,106+i,xOff+64,106+i,0xff);
                Draw_Line_Char(xOff+67,106+i,xOff+67,106+i,0xff); break;
    
        case 47: Draw_Line_Char(0,108+i,127,108+i,0x00);
                Draw_Line_Char(xOff+25,108+i,xOff+25,108+i,0xff);
                Draw_Line_Char(xOff+28,108+i,xOff+34,108+i,0xff);
                Draw_Line_Char(xOff+56,108+i,xOff+63,108+i,0xff);
                Draw_Line_Char(xOff+65,108+i,xOff+65,108+i,0xff); break;
    
        case 48: Draw_Line_Char(0,110+i,127,110+i,0x00);
                Draw_Line_Char(xOff+26,110+i,xOff+26,110+i,0xff);
                Draw_Line_Char(xOff+30,110+i,xOff+36,110+i,0xff);
                Draw_Line_Char(xOff+54,110+i,xOff+61,110+i,0xff);
                Draw_Line_Char(xOff+64,110+i,xOff+64,110+i,0xff); break;
    
        case 49: Draw_Line_Char(0,112+i,127,112+i,0x00);
                Draw_Line_Char(xOff+29,112+i,xOff+29,112+i,0xff);
                Draw_Line_Char(xOff+32,112+i,xOff+37,112+i,0xff);
                Draw_Line_Char(xOff+53,112+i,xOff+57,112+i,0xff);
                Draw_Line_Char(xOff+62,112+i,xOff+62,112+i,0xff); break;
    
        case 50: Draw_Line_Char(0,114+i,127,114+i,0x00);
                Draw_Line_Char(xOff+31,114+i,xOff+31,114+i,0xff);
                Draw_Line_Char(xOff+36,114+i,xOff+39,114+i,0xff);
                Draw_Line_Char(xOff+49,114+i,xOff+55,114+i,0xff);
                Draw_Line_Char(xOff+59,114+i,xOff+59,114+i,0xff); break;
    
        case 51: Draw_Line_Char(0,116+i,127,116+i,0x00);
                Draw_Line_Char(xOff+35,116+i,xOff+36,116+i,0xff);
                Draw_Line_Char(xOff+54,116+i,xOff+56,116+i,0xff); break;
    
        case 52: Draw_Line_Char(0,118+i,127,118+i,0x00);
                Draw_Line_Char(xOff+41,118+i,xOff+50,118+i,0xff); break;
      }
    }
  }
}

void printLogo()
{
  Draw_Line_Char(49,9,49,9,0x25);
//9
  Draw_Line_Char(46,10,46,10,0x25);
  Draw_Line_Char(47,10,49,10,0x96);
  Draw_Line_Char(50,10,50,10,0x25);
//10
  Draw_Line_Char(43,11,43,11,0x25);
  Draw_Line_Char(44,11,49,11,0x96);
  Draw_Line_Char(50,11,50,11,0x25);
//11
  Draw_Line_Char(41,12,41,12,0x25);
  Draw_Line_Char(42,12,50,12,0x96);
//12
  Draw_Line_Char(40,13,50,13,0x96);
//13
  Draw_Line_Char(40,14,40,14,0x25);
  Draw_Line_Char(41,14,50,14,0x96);
//14
  Draw_Line_Char(41,15,50,15,0x96);
  Draw_Line_Char(57,15,59,15,0x25);
//15
  Draw_Line_Char(41,16,41,16,0x25);
  Draw_Line_Char(42,16,50,16,0x96);
  Draw_Line_Char(51,16,53,16,0x25);
  Draw_Line_Char(54,16,59,16,0x96);
//16
  Draw_Line_Char(42,17,59,17,0x96);
//17
  Draw_Line_Char(31,18,32,18,0x96);
  Draw_Line_Char(43,18,59,18,0x96);
//18
  Draw_Line_Char(29,19,29,19,0x25);
  Draw_Line_Char(30,19,32,19,0x96);
  Draw_Line_Char(33,19,33,19,0x25);
  Draw_Line_Char(43,19,59,19,0x96);
//19
  Draw_Line_Char(28,20,33,20,0x96);
  Draw_Line_Char(41,20,41,20,0x25);
  Draw_Line_Char(42,20,59,20,0x96);
//20
  Draw_Line_Char(27,21,33,21,0x96);
  Draw_Line_Char(34,21,34,21,0x25);
  Draw_Line_Char(40,21,59,21,0x96);
//21
  Draw_Line_Char(26,22,34,22,0x96);
  Draw_Line_Char(38,22,38,22,0x25);
  Draw_Line_Char(39,22,59,22,0x96);
  Draw_Line_Char(60,22,60,22,0x25);
//22
  Draw_Line_Char(25,23,35,23,0x96);
  Draw_Line_Char(37,23,55,23,0x96);
  Draw_Line_Char(56,23,58,23,0x25);
//23
  Draw_Line_Char(24,24,51,24,0x96);
  Draw_Line_Char(52,24,53,24,0x25);
//24
  Draw_Line_Char(25,25,48,25,0x96);
  Draw_Line_Char(49,25,49,25,0x25);
//25
  Draw_Line_Char(26,26,46,26,0x96);
  Draw_Line_Char(47,26,47,26,0x25);
//26
  Draw_Line_Char(27,27,27,27,0x25);
  Draw_Line_Char(28,27,44,27,0x96);
  Draw_Line_Char(45,27,45,27,0x25);
//27
  Draw_Line_Char(28,28,28,28,0x25);
  Draw_Line_Char(29,28,42,28,0x96);
  Draw_Line_Char(43,28,43,28,0x25);
  Draw_Line_Char(63,28,64,28,0x25);
//28
  Draw_Line_Char(29,29,40,29,0x96);
  Draw_Line_Char(41,29,41,29,0x25);
  Draw_Line_Char(60,29,60,29,0x6e);
  Draw_Line_Char(61,29,66,29,0xff);
  Draw_Line_Char(67,29,67,29,0x96);
  Draw_Line_Char(98,29,98,29,0x25);
//29
  Draw_Line_Char(28,30,28,30,0x92);
  Draw_Line_Char(29,30,29,30,0xff);
  Draw_Line_Char(30,30,30,30,0x92);
  Draw_Line_Char(31,30,39,30,0x96);
  Draw_Line_Char(40,30,40,30,0x25);
  Draw_Line_Char(52,30,52,30,0x25);
  Draw_Line_Char(53,30,53,30,0x92);
  Draw_Line_Char(54,30,54,30,0xdb);
  Draw_Line_Char(55,30,55,30,0x6e);
  Draw_Line_Char(60,30,60,30,0x25);
  Draw_Line_Char(61,30,66,30,0xff);
  Draw_Line_Char(67,30,67,30,0x25);
  Draw_Line_Char(72,30,72,30,0x96);
  Draw_Line_Char(73,30,73,30,0xdb);
  Draw_Line_Char(74,30,74,30,0x92);
  Draw_Line_Char(75,30,75,30,0x25);
  Draw_Line_Char(97,30,97,30,0x96);
  Draw_Line_Char(98,30,98,30,0xff);
  Draw_Line_Char(99,30,99,30,0x6e);
//30
  Draw_Line_Char(18,31,18,31,0xb7);
  Draw_Line_Char(27,31,27,31,0x6e);
  Draw_Line_Char(28,31,30,31,0xff);
  Draw_Line_Char(31,31,31,31,0x92);
  Draw_Line_Char(32,31,38,31,0x96);
  Draw_Line_Char(50,31,50,31,0x6e);
  Draw_Line_Char(51,31,51,31,0xdb);
  Draw_Line_Char(52,31,54,31,0xff);
  Draw_Line_Char(55,31,55,31,0xb7);
  Draw_Line_Char(61,31,66,31,0xff);
  Draw_Line_Char(72,31,72,31,0x92);
  Draw_Line_Char(73,31,75,31,0xff);
  Draw_Line_Char(76,31,76,31,0xdb);
  Draw_Line_Char(77,31,77,31,0x6e);
  Draw_Line_Char(96,31,96,31,0x96);
  Draw_Line_Char(97,31,99,31,0xff);
  Draw_Line_Char(100,31,100,31,0x96);
  Draw_Line_Char(109,31,109,31,0x92);
//31
  Draw_Line_Char(17,32,17,32,0xb7);
  Draw_Line_Char(18,32,18,32,0xff);
  Draw_Line_Char(19,32,19,32,0xdb);
  Draw_Line_Char(26,32,26,32,0x25);
  Draw_Line_Char(27,32,31,32,0xff);
  Draw_Line_Char(32,32,32,32,0x92);
  Draw_Line_Char(33,32,37,32,0x96);
  Draw_Line_Char(49,32,54,32,0xff);
  Draw_Line_Char(55,32,55,32,0xdb);
  Draw_Line_Char(61,32,61,32,0xdb);
  Draw_Line_Char(62,32,65,32,0xff);
  Draw_Line_Char(66,32,66,32,0xdb);
  Draw_Line_Char(72,32,72,32,0xb7);
  Draw_Line_Char(73,32,78,32,0xff);
  Draw_Line_Char(95,32,95,32,0x96);
  Draw_Line_Char(96,32,100,32,0xff);
  Draw_Line_Char(101,32,101,32,0x25);
  Draw_Line_Char(108,32,108,32,0xdb);
  Draw_Line_Char(109,32,109,32,0xff);
  Draw_Line_Char(110,32,110,32,0x92);
//32
  Draw_Line_Char(16,33,16,33,0x96);
  Draw_Line_Char(17,33,20,33,0xff);
  Draw_Line_Char(21,33,21,33,0x25);
  Draw_Line_Char(26,33,32,33,0xff);
  Draw_Line_Char(33,33,33,33,0x92);
  Draw_Line_Char(34,33,35,33,0x96);
  Draw_Line_Char(36,33,36,33,0x25);
  Draw_Line_Char(49,33,49,33,0xb7);
  Draw_Line_Char(50,33,55,33,0xff);
  Draw_Line_Char(60,33,60,33,0x25);
  Draw_Line_Char(61,33,61,33,0xdb);
  Draw_Line_Char(62,33,65,33,0xff);
  Draw_Line_Char(66,33,66,33,0xdb);
  Draw_Line_Char(67,33,67,33,0x25);
  Draw_Line_Char(72,33,77,33,0xff);
  Draw_Line_Char(78,33,78,33,0x92);
  Draw_Line_Char(94,33,94,33,0x96);
  Draw_Line_Char(95,33,101,33,0xff);
  Draw_Line_Char(107,33,110,33,0xff);
  Draw_Line_Char(111,33,111,33,0x25);
//33
  Draw_Line_Char(16,34,21,34,0xff);
  Draw_Line_Char(22,34,22,34,0x96);
  Draw_Line_Char(25,34,25,34,0xdb);
  Draw_Line_Char(26,34,33,34,0xff);
  Draw_Line_Char(34,34,34,34,0x92);
  Draw_Line_Char(35,34,35,34,0x25);
  Draw_Line_Char(50,34,55,34,0xff);
  Draw_Line_Char(56,34,56,34,0x92);
  Draw_Line_Char(57,34,57,34,0xdb);
  Draw_Line_Char(58,34,69,34,0xff);
  Draw_Line_Char(70,34,70,34,0xb7);
  Draw_Line_Char(71,34,71,34,0x6e);
  Draw_Line_Char(72,34,77,34,0xff);
  Draw_Line_Char(93,34,93,34,0x96);
  Draw_Line_Char(94,34,101,34,0xff);
  Draw_Line_Char(102,34,102,34,0xdb);
  Draw_Line_Char(105,34,105,34,0x25);
  Draw_Line_Char(106,34,111,34,0xff);
//34
  Draw_Line_Char(15,35,15,35,0x92);
  Draw_Line_Char(16,35,22,35,0xff);
  Draw_Line_Char(23,35,23,35,0x6e);
  Draw_Line_Char(24,35,24,35,0xb7);
  Draw_Line_Char(25,35,33,35,0xff);
  Draw_Line_Char(34,35,34,35,0x6e);
  Draw_Line_Char(43,35,44,35,0x92);
  Draw_Line_Char(50,35,50,35,0x6e);
  Draw_Line_Char(51,35,76,35,0xff);
  Draw_Line_Char(77,35,77,35,0x6e);
  Draw_Line_Char(83,35,83,35,0x6e);
  Draw_Line_Char(84,35,84,35,0x92);
  Draw_Line_Char(93,35,93,35,0x96);
  Draw_Line_Char(94,35,102,35,0xff);
  Draw_Line_Char(103,35,103,35,0x92);
  Draw_Line_Char(104,35,104,35,0x96);
  Draw_Line_Char(105,35,111,35,0xff);
  Draw_Line_Char(112,35,112,35,0x6e);
//35
  Draw_Line_Char(15,36,32,36,0xff);
  Draw_Line_Char(33,36,33,36,0x92);
  Draw_Line_Char(42,36,42,36,0xdb);
  Draw_Line_Char(43,36,44,36,0xff);
  Draw_Line_Char(45,36,45,36,0x25);
  Draw_Line_Char(50,36,50,36,0x96);
  Draw_Line_Char(51,36,76,36,0xff);
  Draw_Line_Char(77,36,77,36,0x96);
  Draw_Line_Char(83,36,84,36,0xff);
  Draw_Line_Char(85,36,85,36,0xdb);
  Draw_Line_Char(94,36,94,36,0x6e);
  Draw_Line_Char(95,36,112,36,0xff);
//36
  Draw_Line_Char(14,37,14,37,0xb7);
  Draw_Line_Char(15,37,31,37,0xff);
  Draw_Line_Char(32,37,32,37,0xb7);
  Draw_Line_Char(40,37,40,37,0x25);
  Draw_Line_Char(41,37,44,37,0xff);
  Draw_Line_Char(45,37,45,37,0xb7);
  Draw_Line_Char(48,37,48,37,0x25);
  Draw_Line_Char(49,37,49,37,0xdb);
  Draw_Line_Char(50,37,77,37,0xff);
  Draw_Line_Char(78,37,78,37,0xdb);
  Draw_Line_Char(79,37,79,37,0x25);
  Draw_Line_Char(82,37,82,37,0x92);
  Draw_Line_Char(83,37,85,37,0xff);
  Draw_Line_Char(86,37,86,37,0xdb);
  Draw_Line_Char(87,37,87,37,0x25);
  Draw_Line_Char(95,37,95,37,0x92);
  Draw_Line_Char(96,37,112,37,0xff);
  Draw_Line_Char(113,37,113,37,0xb7);
//37
  Draw_Line_Char(13,38,13,38,0x25);
  Draw_Line_Char(14,38,30,38,0xff);
  Draw_Line_Char(31,38,31,38,0xdb);
  Draw_Line_Char(39,38,39,38,0x25);
  Draw_Line_Char(40,38,45,38,0xff);
  Draw_Line_Char(46,38,46,38,0x96);
  Draw_Line_Char(47,38,47,38,0xb7);
  Draw_Line_Char(48,38,58,38,0xff);
  Draw_Line_Char(59,38,59,38,0xdb);
  Draw_Line_Char(60,38,61,38,0xb7);
  Draw_Line_Char(62,38,62,38,0x92);
  Draw_Line_Char(63,38,64,38,0x6e);
  Draw_Line_Char(65,38,67,38,0x92);
  Draw_Line_Char(68,38,68,38,0xdb);
  Draw_Line_Char(69,38,79,38,0xff);
  Draw_Line_Char(80,38,80,38,0x92);
  Draw_Line_Char(81,38,81,38,0x25);
  Draw_Line_Char(82,38,87,38,0xff);
  Draw_Line_Char(88,38,88,38,0x25);
  Draw_Line_Char(96,38,96,38,0xdb);
  Draw_Line_Char(97,38,113,38,0xff);
//38
  Draw_Line_Char(13,39,13,39,0xb7);
  Draw_Line_Char(14,39,30,39,0xff);
  Draw_Line_Char(39,39,39,39,0x6e);
  Draw_Line_Char(40,39,54,39,0xff);
  Draw_Line_Char(55,39,55,39,0xdb);
  Draw_Line_Char(56,39,56,39,0x92);
  Draw_Line_Char(57,39,57,39,0x96);
  Draw_Line_Char(70,39,70,39,0x25);
  Draw_Line_Char(71,39,71,39,0x6e);
  Draw_Line_Char(72,39,72,39,0xdb);
  Draw_Line_Char(73,39,87,39,0xff);
  Draw_Line_Char(88,39,88,39,0x25);
  Draw_Line_Char(97,39,113,39,0xff);
  Draw_Line_Char(114,39,114,39,0xb7);
//39
  Draw_Line_Char(13,40,29,40,0xff);
  Draw_Line_Char(30,40,30,40,0x96);
  Draw_Line_Char(40,40,40,40,0x6e);
  Draw_Line_Char(41,40,52,40,0xff);
  Draw_Line_Char(53,40,53,40,0xb7);
  Draw_Line_Char(54,40,54,40,0x96);
  Draw_Line_Char(73,40,73,40,0x25);
  Draw_Line_Char(74,40,74,40,0x92);
  Draw_Line_Char(75,40,86,40,0xff);
  Draw_Line_Char(87,40,87,40,0x25);
  Draw_Line_Char(97,40,97,40,0x25);
  Draw_Line_Char(98,40,114,40,0xff);
//40
  Draw_Line_Char(14,41,14,41,0x6e);
  Draw_Line_Char(15,41,15,41,0xdb);
  Draw_Line_Char(16,41,28,41,0xff);
  Draw_Line_Char(29,41,29,41,0xb7);
  Draw_Line_Char(41,41,41,41,0x6e);
  Draw_Line_Char(42,41,50,41,0xff);
  Draw_Line_Char(51,41,51,41,0xb7);
  Draw_Line_Char(52,41,52,41,0x25);
  Draw_Line_Char(76,41,76,41,0xb7);
  Draw_Line_Char(77,41,85,41,0xff);
  Draw_Line_Char(86,41,86,41,0x25);
  Draw_Line_Char(98,41,98,41,0xb7);
  Draw_Line_Char(99,41,111,41,0xff);
  Draw_Line_Char(112,41,112,41,0xdb);
  Draw_Line_Char(113,41,113,41,0x6e);
//41
  Draw_Line_Char(17,42,17,42,0x92);
  Draw_Line_Char(18,42,28,42,0xff);
  Draw_Line_Char(42,42,42,42,0x6e);
  Draw_Line_Char(43,42,48,42,0xff);
  Draw_Line_Char(49,42,49,42,0xdb);
  Draw_Line_Char(50,42,50,42,0x96);
  Draw_Line_Char(77,42,77,42,0x25);
  Draw_Line_Char(78,42,78,42,0xdb);
  Draw_Line_Char(79,42,84,42,0xff);
  Draw_Line_Char(85,42,85,42,0x92);
  Draw_Line_Char(99,42,109,42,0xff);
  Draw_Line_Char(110,42,110,42,0x92);
//42
  Draw_Line_Char(19,43,19,43,0x6e);
  Draw_Line_Char(20,43,27,43,0xff);
  Draw_Line_Char(28,43,28,43,0x6e);
  Draw_Line_Char(35,43,35,43,0x25);
  Draw_Line_Char(41,43,41,43,0x25);
  Draw_Line_Char(42,43,42,43,0x96);
  Draw_Line_Char(43,43,43,43,0x6e);
  Draw_Line_Char(44,43,47,43,0xff);
  Draw_Line_Char(48,43,48,43,0xb7);
  Draw_Line_Char(78,43,78,43,0x25);
  Draw_Line_Char(79,43,85,43,0xff);
  Draw_Line_Char(86,43,86,43,0x96);
  Draw_Line_Char(99,43,99,43,0x6e);
  Draw_Line_Char(100,43,107,43,0xff);
  Draw_Line_Char(108,43,108,43,0x96);
//43
  Draw_Line_Char(19,44,19,44,0xdb);
  Draw_Line_Char(20,44,27,44,0xff);
  Draw_Line_Char(35,44,36,44,0x96);
  Draw_Line_Char(40,44,40,44,0x25);
  Draw_Line_Char(41,44,43,44,0x96);
  Draw_Line_Char(44,44,44,44,0x6e);
  Draw_Line_Char(45,44,46,44,0xff);
  Draw_Line_Char(47,44,47,44,0x92);
  Draw_Line_Char(77,44,77,44,0x25);
  Draw_Line_Char(78,44,78,44,0xdb);
  Draw_Line_Char(79,44,86,44,0xff);
  Draw_Line_Char(87,44,87,44,0x25);
  Draw_Line_Char(100,44,100,44,0xdb);
  Draw_Line_Char(101,44,107,44,0xff);
  Draw_Line_Char(108,44,108,44,0xb7);
//44
  Draw_Line_Char(18,45,18,45,0x25);
  Draw_Line_Char(19,45,26,45,0xff);
  Draw_Line_Char(27,45,27,45,0x96);
  Draw_Line_Char(34,45,37,45,0x96);
  Draw_Line_Char(40,45,44,45,0x96);
  Draw_Line_Char(45,45,46,45,0x6e);
  Draw_Line_Char(76,45,76,45,0x25);
  Draw_Line_Char(77,45,77,45,0xdb);
  Draw_Line_Char(78,45,85,45,0xff);
  Draw_Line_Char(86,45,86,45,0xdb);
  Draw_Line_Char(100,45,100,45,0x25);
  Draw_Line_Char(101,45,108,45,0xff);
//45
  Draw_Line_Char(18,46,18,46,0xb7);
  Draw_Line_Char(19,46,25,46,0xff);
  Draw_Line_Char(26,46,26,46,0xdb);
  Draw_Line_Char(33,46,33,46,0x25);
  Draw_Line_Char(34,46,44,46,0x96);
  Draw_Line_Char(45,46,45,46,0x25);
  Draw_Line_Char(75,46,75,46,0x25);
  Draw_Line_Char(76,46,76,46,0xdb);
  Draw_Line_Char(77,46,84,46,0xff);
  Draw_Line_Char(85,46,85,46,0xdb);
  Draw_Line_Char(101,46,101,46,0xdb);
  Draw_Line_Char(102,46,108,46,0xff);
  Draw_Line_Char(109,46,109,46,0x92);
//46
  Draw_Line_Char(18,47,25,47,0xff);
  Draw_Line_Char(26,47,26,47,0x6e);
  Draw_Line_Char(33,47,43,47,0x96);
  Draw_Line_Char(44,47,44,47,0x25);
  Draw_Line_Char(75,47,75,47,0xb7);
  Draw_Line_Char(76,47,83,47,0xff);
  Draw_Line_Char(84,47,84,47,0xdb);
  Draw_Line_Char(101,47,101,47,0x96);
  Draw_Line_Char(102,47,109,47,0xff);
//47
  Draw_Line_Char(17,48,17,48,0x96);
  Draw_Line_Char(18,48,25,48,0xff);
  Draw_Line_Char(32,48,32,48,0x25);
  Draw_Line_Char(33,48,43,48,0x96);
  Draw_Line_Char(74,48,74,48,0xb7);
  Draw_Line_Char(75,48,82,48,0xff);
  Draw_Line_Char(83,48,83,48,0xdb);
  Draw_Line_Char(102,48,109,48,0xff);
  Draw_Line_Char(110,48,110,48,0x25);
//48
  Draw_Line_Char(9,49,9,49,0x6e);
  Draw_Line_Char(10,49,10,49,0xff);
  Draw_Line_Char(11,49,11,49,0xb7);
  Draw_Line_Char(12,49,12,49,0x25);
  Draw_Line_Char(17,49,17,49,0xb7);
  Draw_Line_Char(18,49,24,49,0xff);
  Draw_Line_Char(25,49,25,49,0xb7);
  Draw_Line_Char(32,49,42,49,0x96);
  Draw_Line_Char(73,49,73,49,0xb7);
  Draw_Line_Char(74,49,81,49,0xff);
  Draw_Line_Char(82,49,82,49,0xdb);
  Draw_Line_Char(102,49,102,49,0x92);
  Draw_Line_Char(103,49,109,49,0xff);
  Draw_Line_Char(110,49,110,49,0x92);
  Draw_Line_Char(116,49,116,49,0x92);
  Draw_Line_Char(117,49,117,49,0xff);
  Draw_Line_Char(118,49,118,49,0x96);
//49
  Draw_Line_Char(9,50,9,50,0xb7);
  Draw_Line_Char(10,50,12,50,0xff);
  Draw_Line_Char(13,50,13,50,0xdb);
  Draw_Line_Char(14,50,14,50,0x92);
  Draw_Line_Char(17,50,24,50,0xff);
  Draw_Line_Char(25,50,25,50,0x25);
  Draw_Line_Char(34,50,34,50,0x25);
  Draw_Line_Char(35,50,41,50,0x96);
  Draw_Line_Char(42,50,42,50,0x25);
  Draw_Line_Char(72,50,72,50,0xb7);
  Draw_Line_Char(73,50,80,50,0xff);
  Draw_Line_Char(81,50,81,50,0xdb);
  Draw_Line_Char(103,50,109,50,0xff);
  Draw_Line_Char(110,50,110,50,0xdb);
  Draw_Line_Char(113,50,113,50,0x6e);
  Draw_Line_Char(114,50,114,50,0xdb);
  Draw_Line_Char(115,50,117,50,0xff);
  Draw_Line_Char(118,50,118,50,0x92);
//50
  Draw_Line_Char(9,51,15,51,0xff);
  Draw_Line_Char(16,51,16,51,0xdb);
  Draw_Line_Char(17,51,23,51,0xff);
  Draw_Line_Char(24,51,24,51,0xdb);
  Draw_Line_Char(36,51,41,51,0x96);
  Draw_Line_Char(71,51,71,51,0xb7);
  Draw_Line_Char(72,51,79,51,0xff);
  Draw_Line_Char(80,51,80,51,0xdb);
  Draw_Line_Char(81,51,81,51,0x25);
  Draw_Line_Char(103,51,103,51,0xdb);
  Draw_Line_Char(104,51,110,51,0xff);
  Draw_Line_Char(111,51,111,51,0xb7);
  Draw_Line_Char(112,51,117,51,0xff);
  Draw_Line_Char(118,51,118,51,0xdb);
//51
  Draw_Line_Char(9,52,23,52,0xff);
  Draw_Line_Char(24,52,24,52,0xb7);
  Draw_Line_Char(36,52,40,52,0x96);
  Draw_Line_Char(70,52,70,52,0x92);
  Draw_Line_Char(71,52,78,52,0xff);
  Draw_Line_Char(79,52,79,52,0xdb);
  Draw_Line_Char(80,52,80,52,0x25);
  Draw_Line_Char(103,52,103,52,0x92);
  Draw_Line_Char(104,52,118,52,0xff);
//52
  Draw_Line_Char(8,53,8,53,0x96);
  Draw_Line_Char(9,53,23,53,0xff);
  Draw_Line_Char(24,53,24,53,0x96);
  Draw_Line_Char(35,53,35,53,0x25);
  Draw_Line_Char(36,53,40,53,0x96);
  Draw_Line_Char(69,53,69,53,0x92);
  Draw_Line_Char(70,53,77,53,0xff);
  Draw_Line_Char(78,53,78,53,0xdb);
  Draw_Line_Char(79,53,79,53,0x25);
  Draw_Line_Char(103,53,103,53,0x25);
  Draw_Line_Char(104,53,118,53,0xff);
  Draw_Line_Char(119,53,119,53,0x25);
//53
  Draw_Line_Char(8,54,8,54,0x92);
  Draw_Line_Char(9,54,23,54,0xff);
  Draw_Line_Char(30,54,30,54,0x25);
  Draw_Line_Char(35,54,39,54,0x96);
  Draw_Line_Char(40,54,40,54,0x25);
  Draw_Line_Char(68,54,68,54,0x92);
  Draw_Line_Char(69,54,76,54,0xff);
  Draw_Line_Char(77,54,77,54,0xdb);
  Draw_Line_Char(78,54,78,54,0x25);
  Draw_Line_Char(104,54,118,54,0xff);
  Draw_Line_Char(119,54,119,54,0x6e);
//54
  Draw_Line_Char(8,55,8,55,0xb7);
  Draw_Line_Char(9,55,22,55,0xff);
  Draw_Line_Char(23,55,23,55,0xdb);
  Draw_Line_Char(30,55,32,55,0x96);
  Draw_Line_Char(33,55,34,55,0x25);
  Draw_Line_Char(35,55,39,55,0x96);
  Draw_Line_Char(67,55,67,55,0x92);
  Draw_Line_Char(68,55,75,55,0xff);
  Draw_Line_Char(76,55,76,55,0xdb);
  Draw_Line_Char(77,55,77,55,0x25);
  Draw_Line_Char(104,55,104,55,0xdb);
  Draw_Line_Char(105,55,118,55,0xff);
  Draw_Line_Char(119,55,119,55,0x92);
//55
  Draw_Line_Char(8,56,8,56,0xdb);
  Draw_Line_Char(9,56,22,56,0xff);
  Draw_Line_Char(23,56,23,56,0xb7);
  Draw_Line_Char(29,56,29,56,0x25);
  Draw_Line_Char(30,56,38,56,0x96);
  Draw_Line_Char(39,56,39,56,0x25);
  Draw_Line_Char(66,56,66,56,0x92);
  Draw_Line_Char(67,56,74,56,0xff);
  Draw_Line_Char(75,56,75,56,0xdb);
  Draw_Line_Char(76,56,76,56,0x25);
  Draw_Line_Char(104,56,104,56,0x92);
  Draw_Line_Char(105,56,118,56,0xff);
  Draw_Line_Char(119,56,119,56,0xdb);
//56
  Draw_Line_Char(8,57,22,57,0xff);
  Draw_Line_Char(23,57,23,57,0x92);
  Draw_Line_Char(29,57,29,57,0x25);
  Draw_Line_Char(30,57,38,57,0x96);
  Draw_Line_Char(65,57,65,57,0x92);
  Draw_Line_Char(66,57,73,57,0xff);
  Draw_Line_Char(74,57,74,57,0xdb);
  Draw_Line_Char(75,57,75,57,0x25);
  Draw_Line_Char(104,57,104,57,0x6e);
  Draw_Line_Char(105,57,118,57,0xff);
  Draw_Line_Char(119,57,119,57,0xdb);
//57
  Draw_Line_Char(8,58,22,58,0xff);
  Draw_Line_Char(23,58,23,58,0x96);
  Draw_Line_Char(29,58,38,58,0x96);
  Draw_Line_Char(64,58,64,58,0x92);
  Draw_Line_Char(65,58,72,58,0xff);
  Draw_Line_Char(73,58,73,58,0xdb);
  Draw_Line_Char(74,58,74,58,0x96);
  Draw_Line_Char(104,58,104,58,0x96);
  Draw_Line_Char(105,58,119,58,0xff);
//58
  Draw_Line_Char(8,59,8,59,0xdb);
  Draw_Line_Char(9,59,14,59,0xb7);
  Draw_Line_Char(15,59,15,59,0xdb);
  Draw_Line_Char(16,59,22,59,0xff);
  Draw_Line_Char(23,59,23,59,0x25);
  Draw_Line_Char(29,59,38,59,0x96);
  Draw_Line_Char(63,59,63,59,0x6e);
  Draw_Line_Char(64,59,71,59,0xff);
  Draw_Line_Char(72,59,72,59,0xdb);
  Draw_Line_Char(73,59,73,59,0x96);
  Draw_Line_Char(105,59,111,59,0xff);
  Draw_Line_Char(112,59,112,59,0xdb);
  Draw_Line_Char(113,59,119,59,0xb7);
//59
  Draw_Line_Char(15,60,15,60,0xb7);
  Draw_Line_Char(16,60,22,60,0xff);
  Draw_Line_Char(29,60,37,60,0x96);
  Draw_Line_Char(38,60,38,60,0x25);
  Draw_Line_Char(62,60,62,60,0x6e);
  Draw_Line_Char(63,60,70,60,0xff);
  Draw_Line_Char(71,60,71,60,0xdb);
  Draw_Line_Char(72,60,72,60,0x96);
  Draw_Line_Char(105,60,111,60,0xff);
  Draw_Line_Char(112,60,112,60,0xb7);
//60
  Draw_Line_Char(15,61,15,61,0xdb);
  Draw_Line_Char(16,61,22,61,0xff);
  Draw_Line_Char(36,61,38,61,0x25);
  Draw_Line_Char(61,61,61,61,0x6e);
  Draw_Line_Char(62,61,70,61,0xff);
  Draw_Line_Char(71,61,71,61,0x96);
  Draw_Line_Char(105,61,111,61,0xff);
  Draw_Line_Char(112,61,112,61,0xdb);
//61
  Draw_Line_Char(15,62,15,62,0xdb);
  Draw_Line_Char(16,62,22,62,0xff);
  Draw_Line_Char(60,62,60,62,0x6e);
  Draw_Line_Char(61,62,69,62,0xff);
  Draw_Line_Char(70,62,70,62,0x96);
  Draw_Line_Char(105,62,111,62,0xff);
  Draw_Line_Char(112,62,112,62,0xdb);
//62
  Draw_Line_Char(15,63,15,63,0xdb);
  Draw_Line_Char(16,63,21,63,0xff);
  Draw_Line_Char(22,63,22,63,0xdb);
  Draw_Line_Char(59,63,59,63,0x6e);
  Draw_Line_Char(60,63,68,63,0xff);
  Draw_Line_Char(69,63,69,63,0x96);
  Draw_Line_Char(105,63,105,63,0xdb);
  Draw_Line_Char(106,63,111,63,0xff);
  Draw_Line_Char(112,63,112,63,0xdb);
//63
  Draw_Line_Char(15,64,15,64,0xdb);
  Draw_Line_Char(16,64,21,64,0xff);
  Draw_Line_Char(22,64,22,64,0xdb);
  Draw_Line_Char(58,64,58,64,0x96);
  Draw_Line_Char(59,64,67,64,0xff);
  Draw_Line_Char(68,64,68,64,0x6e);
  Draw_Line_Char(105,64,105,64,0xdb);
  Draw_Line_Char(106,64,111,64,0xff);
  Draw_Line_Char(112,64,112,64,0xdb);
//64
  Draw_Line_Char(15,65,15,65,0xdb);
  Draw_Line_Char(16,65,22,65,0xff);
  Draw_Line_Char(57,65,57,65,0x96);
  Draw_Line_Char(58,65,66,65,0xff);
  Draw_Line_Char(67,65,67,65,0x6e);
  Draw_Line_Char(105,65,111,65,0xff);
  Draw_Line_Char(112,65,112,65,0xdb);
//65
  Draw_Line_Char(15,66,15,66,0xdb);
  Draw_Line_Char(16,66,22,66,0xff);
  Draw_Line_Char(56,66,56,66,0x96);
  Draw_Line_Char(57,66,65,66,0xff);
  Draw_Line_Char(66,66,66,66,0x6e);
  Draw_Line_Char(90,66,90,66,0x25);
  Draw_Line_Char(105,66,111,66,0xff);
  Draw_Line_Char(112,66,112,66,0xdb);
//66
  Draw_Line_Char(15,67,15,67,0xb7);
  Draw_Line_Char(16,67,22,67,0xff);
  Draw_Line_Char(55,67,55,67,0x96);
  Draw_Line_Char(56,67,64,67,0xff);
  Draw_Line_Char(65,67,65,67,0x6e);
  Draw_Line_Char(89,67,89,67,0x25);
  Draw_Line_Char(90,67,97,67,0x96);
  Draw_Line_Char(98,67,98,67,0x25);
  Draw_Line_Char(105,67,111,67,0xff);
  Draw_Line_Char(112,67,112,67,0xb7);
//67
  Draw_Line_Char(8,68,14,68,0xb7);
  Draw_Line_Char(15,68,15,68,0xdb);
  Draw_Line_Char(16,68,22,68,0xff);
  Draw_Line_Char(54,68,54,68,0x96);
  Draw_Line_Char(55,68,63,68,0xff);
  Draw_Line_Char(64,68,64,68,0x6e);
  Draw_Line_Char(89,68,98,68,0x96);
  Draw_Line_Char(104,68,104,68,0x25);
  Draw_Line_Char(105,68,111,68,0xff);
  Draw_Line_Char(112,68,112,68,0xdb);
  Draw_Line_Char(113,68,118,68,0xb7);
  Draw_Line_Char(119,68,119,68,0xdb);
//68
  Draw_Line_Char(8,69,22,69,0xff);
  Draw_Line_Char(23,69,23,69,0x96);
  Draw_Line_Char(53,69,53,69,0x96);
  Draw_Line_Char(54,69,62,69,0xff);
  Draw_Line_Char(63,69,63,69,0x6e);
  Draw_Line_Char(89,69,98,69,0x96);
  Draw_Line_Char(104,69,104,69,0x96);
  Draw_Line_Char(105,69,119,69,0xff);
//69
  Draw_Line_Char(8,70,8,70,0xdb);
  Draw_Line_Char(9,70,22,70,0xff);
  Draw_Line_Char(23,70,23,70,0x6e);
  Draw_Line_Char(52,70,52,70,0x96);
  Draw_Line_Char(53,70,61,70,0xff);
  Draw_Line_Char(62,70,62,70,0x92);
  Draw_Line_Char(89,70,97,70,0x96);
  Draw_Line_Char(98,70,98,70,0x25);
  Draw_Line_Char(104,70,104,70,0x92);
  Draw_Line_Char(105,70,119,70,0xff);
//70
  Draw_Line_Char(8,71,8,71,0xdb);
  Draw_Line_Char(9,71,22,71,0xff);
  Draw_Line_Char(23,71,23,71,0x92);
  Draw_Line_Char(51,71,51,71,0x96);
  Draw_Line_Char(52,71,60,71,0xff);
  Draw_Line_Char(61,71,61,71,0x92);
  Draw_Line_Char(88,71,88,71,0x25);
  Draw_Line_Char(89,71,97,71,0x96);
  Draw_Line_Char(98,71,98,71,0x25);
  Draw_Line_Char(104,71,104,71,0xb7);
  Draw_Line_Char(105,71,118,71,0xff);
  Draw_Line_Char(119,71,119,71,0xdb);
//71
  Draw_Line_Char(8,72,8,72,0x92);
  Draw_Line_Char(9,72,22,72,0xff);
  Draw_Line_Char(23,72,23,72,0xdb);
  Draw_Line_Char(50,72,50,72,0x96);
  Draw_Line_Char(51,72,59,72,0xff);
  Draw_Line_Char(60,72,60,72,0x92);
  Draw_Line_Char(88,72,92,72,0x96);
  Draw_Line_Char(93,72,93,72,0x25);
  Draw_Line_Char(94,72,97,72,0x96);
  Draw_Line_Char(104,72,104,72,0xdb);
  Draw_Line_Char(105,72,118,72,0xff);
  Draw_Line_Char(119,72,119,72,0xb7);
//72
  Draw_Line_Char(8,73,8,73,0x6e);
  Draw_Line_Char(9,73,23,73,0xff);
  Draw_Line_Char(49,73,49,73,0x25);
  Draw_Line_Char(50,73,58,73,0xff);
  Draw_Line_Char(59,73,59,73,0x92);
  Draw_Line_Char(88,73,92,73,0x96);
  Draw_Line_Char(96,73,96,73,0x25);
  Draw_Line_Char(97,73,97,73,0x96);
  Draw_Line_Char(104,73,118,73,0xff);
  Draw_Line_Char(119,73,119,73,0x92);
//73
  Draw_Line_Char(8,74,8,74,0x25);
  Draw_Line_Char(9,74,23,74,0xff);
  Draw_Line_Char(24,74,24,74,0x25);
  Draw_Line_Char(48,74,48,74,0x25);
  Draw_Line_Char(49,74,57,74,0xff);
  Draw_Line_Char(58,74,58,74,0x92);
  Draw_Line_Char(87,74,87,74,0x25);
  Draw_Line_Char(88,74,92,74,0x96);
  Draw_Line_Char(103,74,103,74,0x96);
  Draw_Line_Char(104,74,118,74,0xff);
  Draw_Line_Char(119,74,119,74,0x96);
//74
  Draw_Line_Char(9,75,23,75,0xff);
  Draw_Line_Char(24,75,24,75,0x92);
  Draw_Line_Char(47,75,47,75,0x25);
  Draw_Line_Char(48,75,56,75,0xff);
  Draw_Line_Char(57,75,57,75,0x92);
  Draw_Line_Char(87,75,91,75,0x96);
  Draw_Line_Char(92,75,92,75,0x25);
  Draw_Line_Char(103,75,103,75,0xb7);
  Draw_Line_Char(104,75,118,75,0xff);
//75
  Draw_Line_Char(9,76,9,76,0xdb);
  Draw_Line_Char(10,76,15,76,0xff);
  Draw_Line_Char(16,76,16,76,0xb7);
  Draw_Line_Char(17,76,23,76,0xff);
  Draw_Line_Char(24,76,24,76,0xdb);
  Draw_Line_Char(46,76,46,76,0x25);
  Draw_Line_Char(47,76,55,76,0xff);
  Draw_Line_Char(56,76,56,76,0xb7);
  Draw_Line_Char(86,76,86,76,0x25);
  Draw_Line_Char(87,76,91,76,0x96);
  Draw_Line_Char(103,76,103,76,0xdb);
  Draw_Line_Char(104,76,110,76,0xff);
  Draw_Line_Char(111,76,111,76,0xdb);
  Draw_Line_Char(112,76,118,76,0xff);
//76
  Draw_Line_Char(9,77,9,77,0x92);
  Draw_Line_Char(10,77,12,77,0xff);
  Draw_Line_Char(13,77,13,77,0xdb);
  Draw_Line_Char(14,77,14,77,0x6e);
  Draw_Line_Char(17,77,17,77,0xdb);
  Draw_Line_Char(18,77,24,77,0xff);
  Draw_Line_Char(45,77,45,77,0x25);
  Draw_Line_Char(46,77,46,77,0xdb);
  Draw_Line_Char(47,77,54,77,0xff);
  Draw_Line_Char(55,77,55,77,0xb7);
  Draw_Line_Char(86,77,91,77,0x96);
  Draw_Line_Char(92,77,92,77,0x25);
  Draw_Line_Char(102,77,102,77,0x25);
  Draw_Line_Char(103,77,110,77,0xff);
  Draw_Line_Char(113,77,113,77,0x92);
  Draw_Line_Char(114,77,114,77,0xdb);
  Draw_Line_Char(115,77,117,77,0xff);
  Draw_Line_Char(118,77,118,77,0xb7);
//77
  Draw_Line_Char(9,78,9,78,0x96);
  Draw_Line_Char(10,78,10,78,0xff);
  Draw_Line_Char(11,78,11,78,0x92);
  Draw_Line_Char(17,78,17,78,0x92);
  Draw_Line_Char(18,78,24,78,0xff);
  Draw_Line_Char(25,78,25,78,0x92);
  Draw_Line_Char(44,78,44,78,0x25);
  Draw_Line_Char(45,78,45,78,0xdb);
  Draw_Line_Char(46,78,53,78,0xff);
  Draw_Line_Char(54,78,54,78,0xb7);
  Draw_Line_Char(85,78,94,78,0x96);
  Draw_Line_Char(95,78,95,78,0x25);
  Draw_Line_Char(102,78,102,78,0xb7);
  Draw_Line_Char(103,78,109,78,0xff);
  Draw_Line_Char(110,78,110,78,0xb7);
  Draw_Line_Char(115,78,115,78,0x25);
  Draw_Line_Char(116,78,116,78,0xb7);
  Draw_Line_Char(117,78,117,78,0xff);
  Draw_Line_Char(118,78,118,78,0x6e);
//78
  Draw_Line_Char(17,79,17,79,0x25);
  Draw_Line_Char(18,79,25,79,0xff);
  Draw_Line_Char(43,79,43,79,0x25);
  Draw_Line_Char(44,79,44,79,0xdb);
  Draw_Line_Char(45,79,52,79,0xff);
  Draw_Line_Char(53,79,53,79,0xb7);
  Draw_Line_Char(84,79,84,79,0x25);
  Draw_Line_Char(85,79,95,79,0x96);
  Draw_Line_Char(102,79,109,79,0xff);
  Draw_Line_Char(110,79,110,79,0x96);
//79
  Draw_Line_Char(18,80,25,80,0xff);
  Draw_Line_Char(26,80,26,80,0x96);
  Draw_Line_Char(42,80,42,80,0x25);
  Draw_Line_Char(43,80,43,80,0xdb);
  Draw_Line_Char(44,80,51,80,0xff);
  Draw_Line_Char(52,80,52,80,0xb7);
  Draw_Line_Char(84,80,94,80,0x96);
  Draw_Line_Char(101,80,101,80,0x6e);
  Draw_Line_Char(102,80,109,80,0xff);
//80
  Draw_Line_Char(18,81,18,81,0x92);
  Draw_Line_Char(19,81,25,81,0xff);
  Draw_Line_Char(26,81,26,81,0xdb);
  Draw_Line_Char(41,81,41,81,0x25);
  Draw_Line_Char(42,81,42,81,0xdb);
  Draw_Line_Char(43,81,50,81,0xff);
  Draw_Line_Char(51,81,51,81,0xb7);
  Draw_Line_Char(83,81,94,81,0x96);
  Draw_Line_Char(101,81,101,81,0xdb);
  Draw_Line_Char(102,81,108,81,0xff);
  Draw_Line_Char(109,81,109,81,0xb7);
//81
  Draw_Line_Char(19,82,26,82,0xff);
  Draw_Line_Char(27,82,27,82,0x25);
  Draw_Line_Char(40,82,40,82,0x25);
  Draw_Line_Char(41,82,41,82,0xdb);
  Draw_Line_Char(42,82,49,82,0xff);
  Draw_Line_Char(50,82,50,82,0xb7);
  Draw_Line_Char(81,82,81,82,0x96);
  Draw_Line_Char(82,82,82,82,0xb7);
  Draw_Line_Char(83,82,87,82,0x96);
  Draw_Line_Char(88,82,89,82,0x25);
  Draw_Line_Char(90,82,93,82,0x96);
  Draw_Line_Char(100,82,100,82,0x96);
  Draw_Line_Char(101,82,108,82,0xff);
  Draw_Line_Char(109,82,109,82,0x25);
//82
  Draw_Line_Char(19,83,19,83,0xb7);
  Draw_Line_Char(20,83,26,83,0xff);
  Draw_Line_Char(27,83,27,83,0xdb);
  Draw_Line_Char(40,83,40,83,0x92);
  Draw_Line_Char(41,83,48,83,0xff);
  Draw_Line_Char(49,83,49,83,0xb7);
  Draw_Line_Char(80,83,80,83,0x96);
  Draw_Line_Char(81,83,82,83,0xff);
  Draw_Line_Char(83,83,83,83,0xb7);
  Draw_Line_Char(84,83,87,83,0x96);
  Draw_Line_Char(91,83,92,83,0x96);
  Draw_Line_Char(93,83,93,83,0x25);
  Draw_Line_Char(100,83,107,83,0xff);
  Draw_Line_Char(108,83,108,83,0xdb);
//83
  Draw_Line_Char(19,84,19,84,0x96);
  Draw_Line_Char(20,84,27,84,0xff);
  Draw_Line_Char(28,84,28,84,0x6e);
  Draw_Line_Char(41,84,41,84,0xb7);
  Draw_Line_Char(42,84,47,84,0xff);
  Draw_Line_Char(48,84,48,84,0xdb);
  Draw_Line_Char(79,84,79,84,0x92);
  Draw_Line_Char(80,84,83,84,0xff);
  Draw_Line_Char(84,84,84,84,0xb7);
  Draw_Line_Char(85,84,86,84,0x96);
  Draw_Line_Char(92,84,92,84,0x96);
  Draw_Line_Char(99,84,99,84,0x6e);
  Draw_Line_Char(100,84,107,84,0xff);
  Draw_Line_Char(108,84,108,84,0x6e);
//84
  Draw_Line_Char(17,85,17,85,0x92);
  Draw_Line_Char(18,85,28,85,0xff);
  Draw_Line_Char(42,85,42,85,0xdb);
  Draw_Line_Char(43,85,48,85,0xff);
  Draw_Line_Char(49,85,49,85,0xdb);
  Draw_Line_Char(50,85,50,85,0x25);
  Draw_Line_Char(77,85,77,85,0x25);
  Draw_Line_Char(78,85,78,85,0xdb);
  Draw_Line_Char(79,85,84,85,0xff);
  Draw_Line_Char(85,85,85,85,0xb7);
  Draw_Line_Char(99,85,109,85,0xff);
  Draw_Line_Char(110,85,110,85,0x92);
//85
  Draw_Line_Char(14,86,14,86,0x6e);
  Draw_Line_Char(15,86,15,86,0xdb);
  Draw_Line_Char(16,86,28,86,0xff);
  Draw_Line_Char(29,86,29,86,0xb7);
  Draw_Line_Char(41,86,41,86,0x92);
  Draw_Line_Char(42,86,50,86,0xff);
  Draw_Line_Char(51,86,51,86,0x92);
  Draw_Line_Char(76,86,76,86,0xb7);
  Draw_Line_Char(77,86,85,86,0xff);
  Draw_Line_Char(86,86,86,86,0x92);
  Draw_Line_Char(98,86,98,86,0xb7);
  Draw_Line_Char(99,86,111,86,0xff);
  Draw_Line_Char(112,86,112,86,0xdb);
  Draw_Line_Char(113,86,113,86,0x6e);
//86
  Draw_Line_Char(13,87,29,87,0xff);
  Draw_Line_Char(30,87,30,87,0x25);
  Draw_Line_Char(40,87,40,87,0x92);
  Draw_Line_Char(41,87,52,87,0xff);
  Draw_Line_Char(53,87,53,87,0x92);
  Draw_Line_Char(54,87,54,87,0x25);
  Draw_Line_Char(73,87,73,87,0x25);
  Draw_Line_Char(74,87,74,87,0xb7);
  Draw_Line_Char(75,87,86,87,0xff);
  Draw_Line_Char(87,87,87,87,0x92);
  Draw_Line_Char(97,87,97,87,0x96);
  Draw_Line_Char(98,87,114,87,0xff);
//87
  Draw_Line_Char(13,88,13,88,0xb7);
  Draw_Line_Char(14,88,30,88,0xff);
  Draw_Line_Char(39,88,39,88,0x6e);
  Draw_Line_Char(40,88,54,88,0xff);
  Draw_Line_Char(55,88,55,88,0xdb);
  Draw_Line_Char(56,88,56,88,0x6e);
  Draw_Line_Char(57,88,57,88,0x25);
  Draw_Line_Char(70,88,70,88,0x25);
  Draw_Line_Char(71,88,71,88,0x92);
  Draw_Line_Char(72,88,72,88,0xdb);
  Draw_Line_Char(73,88,87,88,0xff);
  Draw_Line_Char(88,88,88,88,0x92);
  Draw_Line_Char(97,88,113,88,0xff);
  Draw_Line_Char(114,88,114,88,0xb7);
//88
  Draw_Line_Char(14,89,30,89,0xff);
  Draw_Line_Char(31,89,31,89,0xdb);
  Draw_Line_Char(39,89,39,89,0x25);
  Draw_Line_Char(40,89,45,89,0xff);
  Draw_Line_Char(46,89,46,89,0x25);
  Draw_Line_Char(47,89,47,89,0xb7);
  Draw_Line_Char(48,89,57,89,0xff);
  Draw_Line_Char(58,89,58,89,0xdb);
  Draw_Line_Char(59,89,59,89,0xb7);
  Draw_Line_Char(60,89,60,89,0x92);
  Draw_Line_Char(61,89,62,89,0x6e);
  Draw_Line_Char(63,89,64,89,0x96);
  Draw_Line_Char(65,89,65,89,0x6e);
  Draw_Line_Char(66,89,66,89,0x92);
  Draw_Line_Char(67,89,67,89,0xb7);
  Draw_Line_Char(68,89,68,89,0xdb);
  Draw_Line_Char(69,89,79,89,0xff);
  Draw_Line_Char(80,89,80,89,0xb7);
  Draw_Line_Char(81,89,81,89,0x6e);
  Draw_Line_Char(82,89,87,89,0xff);
  Draw_Line_Char(88,89,88,89,0x6e);
  Draw_Line_Char(96,89,96,89,0xdb);
  Draw_Line_Char(97,89,113,89,0xff);
  Draw_Line_Char(114,89,114,89,0x25);
//89
  Draw_Line_Char(14,90,14,90,0xb7);
  Draw_Line_Char(15,90,31,90,0xff);
  Draw_Line_Char(32,90,32,90,0x92);
  Draw_Line_Char(40,90,40,90,0x25);
  Draw_Line_Char(41,90,41,90,0xdb);
  Draw_Line_Char(42,90,44,90,0xff);
  Draw_Line_Char(45,90,45,90,0x92);
  Draw_Line_Char(48,90,48,90,0x25);
  Draw_Line_Char(49,90,49,90,0xdb);
  Draw_Line_Char(50,90,77,90,0xff);
  Draw_Line_Char(78,90,78,90,0xdb);
  Draw_Line_Char(79,90,79,90,0x6e);
  Draw_Line_Char(82,90,82,90,0xb7);
  Draw_Line_Char(83,90,86,90,0xff);
  Draw_Line_Char(87,90,87,90,0x96);
  Draw_Line_Char(95,90,95,90,0xb7);
  Draw_Line_Char(96,90,112,90,0xff);
  Draw_Line_Char(113,90,113,90,0xb7);
//90
  Draw_Line_Char(15,91,32,91,0xff);
  Draw_Line_Char(33,91,33,91,0x6e);
  Draw_Line_Char(42,91,42,91,0xdb);
  Draw_Line_Char(43,91,44,91,0xff);
  Draw_Line_Char(50,91,50,91,0x96);
  Draw_Line_Char(51,91,76,91,0xff);
  Draw_Line_Char(77,91,77,91,0x92);
  Draw_Line_Char(82,91,82,91,0x25);
  Draw_Line_Char(83,91,84,91,0xff);
  Draw_Line_Char(85,91,85,91,0xdb);
  Draw_Line_Char(86,91,86,91,0x25);
  Draw_Line_Char(94,91,94,91,0x92);
  Draw_Line_Char(95,91,112,91,0xff);
//91
  Draw_Line_Char(15,92,15,92,0x6e);
  Draw_Line_Char(16,92,22,92,0xff);
  Draw_Line_Char(23,92,23,92,0x96);
  Draw_Line_Char(24,92,24,92,0x92);
  Draw_Line_Char(25,92,33,92,0xff);
  Draw_Line_Char(34,92,34,92,0x96);
  Draw_Line_Char(43,92,43,92,0x92);
  Draw_Line_Char(44,92,44,92,0x6e);
  Draw_Line_Char(50,92,50,92,0x96);
  Draw_Line_Char(51,92,76,92,0xff);
  Draw_Line_Char(77,92,77,92,0x96);
  Draw_Line_Char(83,92,84,92,0xb7);
  Draw_Line_Char(93,92,93,92,0x6e);
  Draw_Line_Char(94,92,102,92,0xff);
  Draw_Line_Char(103,92,103,92,0xb7);
  Draw_Line_Char(104,92,104,92,0x6e);
  Draw_Line_Char(105,92,111,92,0xff);
  Draw_Line_Char(112,92,112,92,0x92);
//92
  Draw_Line_Char(16,93,21,93,0xff);
  Draw_Line_Char(22,93,22,93,0x25);
  Draw_Line_Char(25,93,25,93,0xdb);
  Draw_Line_Char(26,93,33,93,0xff);
  Draw_Line_Char(34,93,34,93,0x96);
  Draw_Line_Char(50,93,55,93,0xff);
  Draw_Line_Char(56,93,56,93,0x92);
  Draw_Line_Char(57,93,57,93,0xdb);
  Draw_Line_Char(58,93,69,93,0xff);
  Draw_Line_Char(70,93,70,93,0xdb);
  Draw_Line_Char(71,93,71,93,0xb7);
  Draw_Line_Char(72,93,77,93,0xff);
  Draw_Line_Char(92,93,92,93,0x25);
  Draw_Line_Char(93,93,93,93,0x92);
  Draw_Line_Char(94,93,101,93,0xff);
  Draw_Line_Char(102,93,102,93,0xdb);
  Draw_Line_Char(105,93,105,93,0x96);
  Draw_Line_Char(106,93,111,93,0xff);
//93
  Draw_Line_Char(16,94,16,94,0x25);
  Draw_Line_Char(17,94,20,94,0xff);
  Draw_Line_Char(26,94,32,94,0xff);
  Draw_Line_Char(33,94,33,94,0x96);
  Draw_Line_Char(49,94,49,94,0x92);
  Draw_Line_Char(50,94,55,94,0xff);
  Draw_Line_Char(59,94,59,94,0x25);
  Draw_Line_Char(60,94,60,94,0x96);
  Draw_Line_Char(61,94,61,94,0xdb);
  Draw_Line_Char(62,94,65,94,0xff);
  Draw_Line_Char(66,94,66,94,0xdb);
  Draw_Line_Char(67,94,67,94,0x6e);
  Draw_Line_Char(68,94,68,94,0x96);
  Draw_Line_Char(72,94,77,94,0xff);
  Draw_Line_Char(78,94,78,94,0x92);
  Draw_Line_Char(91,94,91,94,0x25);
  Draw_Line_Char(92,94,93,94,0x96);
  Draw_Line_Char(94,94,94,94,0x92);
  Draw_Line_Char(95,94,101,94,0xff);
  Draw_Line_Char(106,94,106,94,0x25);
  Draw_Line_Char(107,94,110,94,0xff);
  Draw_Line_Char(111,94,111,94,0x96);
//94
  Draw_Line_Char(17,95,17,95,0x92);
  Draw_Line_Char(18,95,18,95,0xff);
  Draw_Line_Char(19,95,19,95,0xdb);
  Draw_Line_Char(26,95,26,95,0x25);
  Draw_Line_Char(27,95,31,95,0xff);
  Draw_Line_Char(32,95,32,95,0x96);
  Draw_Line_Char(49,95,54,95,0xff);
  Draw_Line_Char(55,95,55,95,0xb7);
  Draw_Line_Char(61,95,61,95,0xdb);
  Draw_Line_Char(62,95,65,95,0xff);
  Draw_Line_Char(66,95,66,95,0xdb);
  Draw_Line_Char(72,95,72,95,0xdb);
  Draw_Line_Char(73,95,78,95,0xff);
  Draw_Line_Char(90,95,94,95,0x96);
  Draw_Line_Char(95,95,95,95,0x92);
  Draw_Line_Char(96,95,100,95,0xff);
  Draw_Line_Char(101,95,101,95,0x25);
  Draw_Line_Char(108,95,108,95,0xdb);
  Draw_Line_Char(109,95,109,95,0xff);
  Draw_Line_Char(110,95,110,95,0xb7);
//95
  Draw_Line_Char(18,96,18,96,0x92);
  Draw_Line_Char(27,96,27,96,0x96);
  Draw_Line_Char(28,96,30,96,0xff);
  Draw_Line_Char(31,96,31,96,0x96);
  Draw_Line_Char(50,96,50,96,0x6e);
  Draw_Line_Char(51,96,51,96,0xdb);
  Draw_Line_Char(52,96,54,96,0xff);
  Draw_Line_Char(55,96,55,96,0x92);
  Draw_Line_Char(61,96,66,96,0xff);
  Draw_Line_Char(72,96,72,96,0xb7);
  Draw_Line_Char(73,96,75,96,0xff);
  Draw_Line_Char(76,96,76,96,0xdb);
  Draw_Line_Char(77,96,77,96,0x92);
  Draw_Line_Char(78,96,78,96,0x25);
  Draw_Line_Char(89,96,95,96,0x96);
  Draw_Line_Char(96,96,96,96,0x92);
  Draw_Line_Char(97,96,99,96,0xff);
  Draw_Line_Char(100,96,100,96,0x6e);
  Draw_Line_Char(109,96,109,96,0xb7);
//96
  Draw_Line_Char(28,97,28,97,0x6e);
  Draw_Line_Char(29,97,29,97,0xff);
  Draw_Line_Char(30,97,30,97,0x96);
  Draw_Line_Char(52,97,52,97,0x25);
  Draw_Line_Char(53,97,53,97,0x92);
  Draw_Line_Char(54,97,54,97,0xdb);
  Draw_Line_Char(55,97,55,97,0x96);
  Draw_Line_Char(60,97,60,97,0x25);
  Draw_Line_Char(61,97,66,97,0xff);
  Draw_Line_Char(67,97,67,97,0x25);
  Draw_Line_Char(72,97,72,97,0x92);
  Draw_Line_Char(73,97,73,97,0xdb);
  Draw_Line_Char(74,97,74,97,0xb7);
  Draw_Line_Char(75,97,75,97,0x96);
  Draw_Line_Char(87,97,87,97,0x25);
  Draw_Line_Char(88,97,96,97,0x96);
  Draw_Line_Char(97,97,97,97,0x92);
  Draw_Line_Char(98,97,98,97,0xff);
  Draw_Line_Char(99,97,99,97,0x92);
//97
  Draw_Line_Char(29,98,29,98,0x25);
  Draw_Line_Char(60,98,60,98,0x96);
  Draw_Line_Char(61,98,66,98,0xff);
  Draw_Line_Char(67,98,67,98,0x96);
  Draw_Line_Char(86,98,86,98,0x25);
  Draw_Line_Char(87,98,98,98,0x96);
//98
  Draw_Line_Char(61,99,62,99,0x25);
  Draw_Line_Char(63,99,64,99,0x96);
  Draw_Line_Char(65,99,66,99,0x25);
  Draw_Line_Char(84,99,84,99,0x25);
  Draw_Line_Char(85,99,98,99,0x96);
  Draw_Line_Char(99,99,99,99,0x25);
//99
  Draw_Line_Char(82,100,82,100,0x25);
  Draw_Line_Char(83,100,99,100,0x96);
  Draw_Line_Char(100,100,100,100,0x25);
//100
  Draw_Line_Char(80,101,80,101,0x25);
  Draw_Line_Char(81,101,101,101,0x96);
//101
  Draw_Line_Char(78,102,78,102,0x25);
  Draw_Line_Char(79,102,102,102,0x96);
//102
  Draw_Line_Char(74,103,75,103,0x25);
  Draw_Line_Char(76,103,103,103,0x96);
//103
  Draw_Line_Char(69,104,71,104,0x25);
  Draw_Line_Char(72,104,90,104,0x96);
  Draw_Line_Char(92,104,102,104,0x96);
//104
  Draw_Line_Char(67,105,67,105,0x25);
  Draw_Line_Char(68,105,88,105,0x96);
  Draw_Line_Char(89,105,89,105,0x25);
  Draw_Line_Char(93,105,101,105,0x96);
//105
  Draw_Line_Char(68,106,87,106,0x96);
  Draw_Line_Char(93,106,93,106,0x25);
  Draw_Line_Char(94,106,100,106,0x96);
//106
  Draw_Line_Char(68,107,85,107,0x96);
  Draw_Line_Char(86,107,86,107,0x25);
  Draw_Line_Char(94,107,99,107,0x96);
//107
  Draw_Line_Char(68,108,84,108,0x96);
  Draw_Line_Char(94,108,94,108,0x25);
  Draw_Line_Char(95,108,97,108,0x96);
  Draw_Line_Char(98,108,98,108,0x25);
//108
  Draw_Line_Char(68,109,84,109,0x96);
  Draw_Line_Char(95,109,96,109,0x96);
//109
  Draw_Line_Char(68,110,85,110,0x96);
//110
  Draw_Line_Char(68,111,73,111,0x96);
  Draw_Line_Char(74,111,76,111,0x25);
  Draw_Line_Char(77,111,85,111,0x96);
  Draw_Line_Char(86,111,86,111,0x25);
//111
  Draw_Line_Char(68,112,70,112,0x25);
  Draw_Line_Char(77,112,86,112,0x96);
//112
  Draw_Line_Char(77,113,86,113,0x96);
  Draw_Line_Char(87,113,87,113,0x25);
//113
  Draw_Line_Char(77,114,87,114,0x96);
//114
  Draw_Line_Char(77,115,85,115,0x96);
  Draw_Line_Char(86,115,86,115,0x25);
//115
  Draw_Line_Char(77,116,77,116,0x25);
  Draw_Line_Char(78,116,83,116,0x96);
  Draw_Line_Char(84,116,84,116,0x25);
//116
  Draw_Line_Char(77,117,77,117,0x25);
  Draw_Line_Char(78,117,80,117,0x96);
  Draw_Line_Char(81,117,81,117,0x25);
//117
  Draw_Line_Char(78,118,78,118,0x25);
}

void setup()
{
  long i;
  // set the digital pin as output:
  pinMode(RST_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  pinMode(DC_PIN, OUTPUT);
  pinMode(DIN_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(HVEN_PIN, OUTPUT);
  
  Serial.begin(9600);

  OLED_HVEN_1; //+12V on
  OLED_InitIOs();
  OLED_Init();

  Clear_Window(0x00,0x83,0x00,0x83);
  delay(10); 

  Serial.print("OLED_FRONT_PAN\n");
}

void printSpinner(int i)
{
  int x = 38*cos(i * 0.017452778)+64;
  int y = 38*sin(i * 0.017452778)+64;
  Draw_Circle64k(x, y, 2, 0x00, 0x00);
  x = 38*cos((i+60) * 0.017452778)+64;
  y = 38*sin((i+60) * 0.017452778)+64;
  Draw_Circle64k(x, y, 2,  MAKECOLOR64k(1,1,255), MAKECOLOR64k(1,1,99));
  x = 38*cos((i+120) * 0.017452778)+64;
  y = 38*sin((i+120) * 0.017452778)+64;
  Draw_Circle64k(x, y, 2, 0x00, 0x00);
  x = 38*cos((i+180) * 0.017452778)+64;
  y = 38*sin((i+180) * 0.017452778)+64;
  Draw_Circle64k(x, y, 2, MAKECOLOR64k(11,11,255), MAKECOLOR64k(99,99,255));
  x = 38*cos((i+240) * 0.017452778)+64;
  y = 38*sin((i+240) * 0.017452778)+64;
  Draw_Circle64k(x, y, 2, 0x00, 0x00);
  x = 38*cos((i+300) * 0.017452778)+64;
  y = 38*sin((i+300) * 0.017452778)+64;
  Draw_Circle64k(x, y, 2, MAKECOLOR64k(111,111,255), MAKECOLOR64k(188,188,255));
}

// do put code in loop func
void loop()
{
  int tick = 0; //keep track for sleep mode
  bool useTekLogo = false;
  bool screenRest = false;
  printLogo();
  char in;
  bool timerEn = true;
  bool randomEn = true;


  EnableFill(0x01);

  while(1)
  {
    for(int c = 0; c < (useTekLogo ? 1 : 10); c++) //~1 min of animation
    {
      if((tick > -1) && !screenRest)
      {
        for(int i = 0; i < 720; i += 2)
        {
          if(useTekLogo)
          {
            printTek(i);
          }
          else
          {
            printSpinner(i);
          }
          if(Serial.available()) { break; }
        }
      }
      else
      {
        for(int i = 0; i < 6; i++)
        {
          delay(1000);
          if(Serial.available()) { break; }
        }
      }
      if(Serial.available()) { break; }
    }
    
    if(Serial.available())
    {
      in = Serial.read();
      
      while((in != '>')&&(Serial.available()))
      {
        in = Serial.read();
      }

      if((Serial.available()))
      {
        in = Serial.read();
      
        switch(in)
        {
        case '1':  
          Clear_Window(0x00,0x83,0x00,0x83);
          delay(10);
          Write_Command(0xAE); //disp off
          tick = -1;
          randomEn = false;
          screenRest = false; //not the same as off
          break;
        case '2':
          timerEn = true;
          randomEn = false;
          screenRest = false;
          break;
        case '3':
          tick = 1;
          Write_Command(0xAF); //disp on
          timerEn = false;
          randomEn = false;
          screenRest = false;
          break;
        case 'T':
          tick = 1;
          Write_Command(0xAF); //disp on
          timerEn = true;
          randomEn = false;
          screenRest = false;
   
          Clear_Window(0x00,0x83,0x00,0x83);
          delay(10);
          useTekLogo = true;
          printTek(0);
          break;
        case 'L':
          tick = 1;
          Write_Command(0xAF); //disp on
          timerEn = true;
          randomEn = false;
          screenRest = false;
          
          Clear_Window(0x00,0x83,0x00,0x83);
          delay(10);
          useTekLogo = false;
          printLogo();
          break;
        case 'R':
          tick = 1;
          Write_Command(0xAF); //disp on
          timerEn = true;
          randomEn = true;
          screenRest = false;
          
          Clear_Window(0x00,0x83,0x00,0x83);
          delay(10);
          useTekLogo = false;
          printLogo();
          break;
        case 'I': Serial.print("OLED_FRONT_PAN\n");
          break;
        default: 
          Serial.print("Command Guide:\n");
          Serial.print("\"->1\": display off.\n");
          Serial.print("\"->2\": 1 hr sleep timer.\n");
          Serial.print("\"->3\": display on.\n");
          Serial.print("\"->L\": personal logo.\n");
          Serial.print("\"->T\": TEK logo.\n");
          Serial.print("\"->R\": random (logo/TEK/off) every ~hr.\n");
        }
      }
    }
    
    if(timerEn)
    {
      tick = (tick + 1)%70;
      if(tick == 60) //1hr
      {
        if(randomEn)
        {
          int m = random(0,7);
          screenRest = (m == 0);
          useTekLogo = (m%2 == 0);
          
          if(screenRest)
          {
            Clear_Window(0x00,0x83,0x00,0x83);
            delay(10);
            Write_Command(0xAE); //disp off
          }
          else
          {
            Write_Command(0xAF); //disp on
            if(!useTekLogo)
            {
              Clear_Window(0x00,0x83,0x00,0x83);
              delay(10);
              printLogo();
            }
            else
            {
              Clear_Window(0x00,0x83,0x00,0x83);
              delay(10);
              printTek(0);
            }
          }
        }
        else
        {
          Clear_Window(0x00,0x83,0x00,0x83);
          delay(10);
          Write_Command(0xAE); //disp off
          tick = -1;
        }
      }
    }
  }
}

