/*
 * Board: Maple mini form STM32F1 Boards (STM32Duino.com)
 * Use libusb-win32 driver for Maple 003/Maple DFU Device(bootloader mode)
 * maple_mini_boot20.bin bootloader 2.0
 * 
 * LCD SSD1283A
 * LED ->  -> Maple mini pin 
 * SCK -> PA5 -> Maple mini pin 6
 * SDA -> PA7 -> Maple mini pin 4
 * A0  -> PB7 -> Maple mini pin 15
 * RST -> PA1 -> Maple mini pin 10 
 * CS  -> PA4 -> Maple mini pin 7
 * 
 * DHT22
 * Data pin -> PA8 -> Maple mini pin 27
 * 
 * Temperature
 */

#include "DHT.h"
#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_SPI.h> //Hardware-specific library
#include <EEPROM.h>
#include <RTClock.h>
#include <Wire.h>    // for I2C

#define i2caddr 0x50
#define STM_I2C2_FREQ 400000
TwoWire Wire2 (2,I2C_FAST_MODE);
#define Wire Wire2

#define DHTPIN PA8
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

RTClock rt (RTCSEL_LSI); // initialise


//LCDWIKI_SPI mylcd(SSD1283A,10,9,8,-1); //Pro Mini
//LCDWIKI_SPI mylcd(SSD1283A,15,16,4,-1); //ESP8266

LCDWIKI_SPI mylcd(SSD1283A,PA4,PB7,PA1,-1); //Maple mini //hardware spi,cs,cd,reset,led
//LCDWIKI_SPI mylcd(SSD1283A,PA4,PB7,PA6,PA7,PA1,PA5,-1);//software spi,model,cs,cd,miso,mosi,reset,clk,led


#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define EEPROM_TRIP  0
#define GEAR_POS_X 82
#define GEAR_POS_Y 22
#define GEAR_SIZE_X 46
#define GEAR_SIZE_Y 46
#define GEAR_SIZE_FONT 5
#define SHIFT_LIGHT_RPM 6000

int16_t engineTempPOSX = 50;
int16_t engineTempPOSY = 8;
int16_t externalTempPOSX = 118;
int16_t externalTempPOSY = 5;
int16_t dateTimePOSX = 16;
int16_t dateTimePOSY = 24;
int16_t odometrPOSX = 2;
int16_t odometrPOSY = 84;
int16_t tripPOSX = 116;
int16_t tripPOSY = 104;

int16_t lastEngineTemp;
int16_t lastTemp;
int16_t lastGear;
int16_t rpm;
String lastTrip;
String lastOdometr;
int lastTime;
String lastDate;


float i = -20;
float g = 0;
//long EEPROMReadlong(long address) {
//  long four = EEPROM.read(address);
//  long three = EEPROM.read(address + 1);
//  long two = EEPROM.read(address + 2);
//  long one = EEPROM.read(address + 3);
// 
//  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
//}
//unsigned int trip = EEPROMReadlong(EEPROM_TRIP);


void setup() 
{
  pinMode(PB1, OUTPUT);
  Serial.begin(9600);
  delay(1000);
  Wire.begin();         // wake up the I2C
  dht.begin();
  mylcd.Init_LCD();
  mylcd.Set_Rotation(1);
  mylcd.Set_Text_Mode(1);
  mylcd.Fill_Screen(WHITE);
  //animation();
  initTemp();
  //initGear();
}

void loop() {
  digitalWrite(PB1, HIGH);
  setEngineTemp(i);
  i++;
  if (i==120) i=0;
  setExternalTemp();
//    setGear(g);
  setDate("20/09/2019");
  setTime(20,20,20);
  setOdometr(readTotalOdometr());
  setTrip(readTripOdometr());
//    //EEPROMWritelong(EEPROM_TRIP, trip);
//    g++;
//    
//    if (g==7) g=0;
//    //mylcd.Invert_Display(1);
    delay(100);
    digitalWrite(PB1, LOW);
    delay(100);
  }
  void animation()
  {
    mylcd.Set_Text_colour(BLACK);
    mylcd.Set_Text_Size(3);
    mylcd.Print_String("HONDA", 20, GEAR_POS_Y+GEAR_SIZE_Y-18);
    mylcd.Set_Text_Size(2);
    mylcd.Print_String("VARADERO", 16, GEAR_POS_Y+GEAR_SIZE_Y+14);
    delay(2000);
    mylcd.Set_Draw_color(WHITE);
    mylcd.Fill_Rectangle(2,GEAR_POS_Y+18, 128, 128);
    
  }
  
void setTrip(String km)
{
  if(lastTrip != km){
    lastTrip = km;
    String kmInt = km.substring(0, km.length() - 1);
    String kmDec = km.substring(km.length(), km.length()-1);
    short offset = (kmInt.length()*20);
    mylcd.Set_Draw_color(WHITE);
    mylcd.Set_Text_colour(BLACK);
    mylcd.Fill_Rectangle(tripPOSX-offset, tripPOSY, tripPOSX, tripPOSY+13);
    mylcd.Set_Text_Size(1);
    mylcd.Print_String("TRIP", 1, tripPOSY);
    
    mylcd.Set_Draw_color(BLACK);
    mylcd.Fill_Rectangle(tripPOSX-6, tripPOSY-2, tripPOSX+12, tripPOSY+24);
    mylcd.Set_Text_Size(3);
    mylcd.Print_String(kmInt, tripPOSX-offset, tripPOSY);
    mylcd.Set_Text_colour(WHITE);
    mylcd.Print_String(kmDec, tripPOSX-4, tripPOSY+1);
  }
}
  
  void setOdometr(String km)
  {
    if(lastOdometr != km){
      lastOdometr = km;
      
    String kmInt = km.substring(0, km.length() - 3);
    String kmDec = km.substring(km.length()-2, km.length() - 1);
    short offset = (kmInt.length()*15);
    
    mylcd.Set_Draw_color(WHITE);
    mylcd.Fill_Rectangle(odometrPOSX-offset, odometrPOSY-2, odometrPOSX+125, odometrPOSY+15);
    mylcd.Set_Text_colour(BLACK);
    mylcd.Set_Text_Size(2);
    
    mylcd.Print_String(kmInt, 130-offset-2, odometrPOSY);
    
    //mylcd.Print_String(kmInt, odometrPOSX+2, odometrPOSY);
    
    mylcd.Set_Draw_color(BLACK);
    mylcd.Fill_Rectangle(130-(16), odometrPOSY-1, 130-(2), odometrPOSY+15);   
    mylcd.Set_Text_colour(WHITE);     
    mylcd.Print_String(kmDec, 130-14, odometrPOSY);
    }
    
  }
  
  void setDate(String curDate)
  {
    if(lastDate != curDate){
       lastDate = curDate;
    mylcd.Set_Draw_color(WHITE);
    mylcd.Fill_Rectangle(dateTimePOSX-10, dateTimePOSY, dateTimePOSX+120, dateTimePOSY+14);
    mylcd.Set_Text_colour(BLACK);
    mylcd.Set_Text_Size(2);
    mylcd.Print_String(String(curDate), dateTimePOSX-10, dateTimePOSY);
    }
  }
  
  void setTime(int16_t hour, int16_t minute, int16_t second)
  {
    if(lastTime != hour+minute+second){
       lastTime = hour+minute+second;
    mylcd.Set_Draw_color(WHITE);
    mylcd.Fill_Rectangle(dateTimePOSX, dateTimePOSY+20, dateTimePOSX+98, dateTimePOSY+48);
    mylcd.Set_Text_colour(BLACK);
    mylcd.Set_Text_Size(4);
    mylcd.Print_String(String(hour), dateTimePOSX, dateTimePOSY+20);
    mylcd.Print_String(":", dateTimePOSX+40, dateTimePOSY+20);
    mylcd.Print_String(String(minute), dateTimePOSX+55, dateTimePOSY+20);
    //mylcd.Print_String(":", dateTimePOSX+52, 34);
    //mylcd.Print_String(String(second), dateTimePOSX+54, dateTimePOSY+20);
    }
    
  }
  
  void initGear()
  {
    mylcd.Set_Draw_color(BLACK);
    mylcd.Draw_Rectangle(GEAR_POS_X, GEAR_POS_Y, GEAR_POS_X+GEAR_SIZE_X, GEAR_POS_Y+GEAR_SIZE_Y);
  }
  
  void setGear(int16_t gear)
  {
    if(lastGear != gear){
      lastGear = gear;
      if(rpm>SHIFT_LIGHT_RPM){
        mylcd.Set_Draw_color(RED);
      }
      else{
        mylcd.Set_Draw_color(WHITE);
      }
      mylcd.Fill_Rectangle(GEAR_POS_X+1, GEAR_POS_Y+1, GEAR_POS_X+GEAR_SIZE_X-1, GEAR_POS_Y+GEAR_SIZE_Y-1);
      mylcd.Set_Text_colour(BLACK);
      //mylcd.Set_Text_Size(1);
      //mylcd.Print_String("GEAR", 95, 22);
      mylcd.Set_Text_Size(GEAR_SIZE_FONT);
      if(gear==0){
        mylcd.Print_String("N", GEAR_POS_X+12, GEAR_POS_Y+6);
      }
      else {
        mylcd.Print_String(String(gear), GEAR_POS_X+12, GEAR_POS_Y+6);
      } 
    }
  }
  
  void initTemp()
  {
    mylcd.Set_Draw_color(BLACK);
    mylcd.Draw_Rectangle(2, 2, 128, 20);
    mylcd.Draw_Rectangle(82, 2, 128, 20);
  }

  void setEngineTemp(int16_t curTemp)
  {
    if(lastEngineTemp != curTemp){
      lastEngineTemp = curTemp;
      //clear screan
    mylcd.Set_Draw_color(WHITE);
    mylcd.Fill_Rectangle(4, 4, 80, 18);
    if (curTemp > 100){
      mylcd.Set_Draw_color(RED);
    }
    else{
      mylcd.Set_Draw_color(BLACK);
    }
    mylcd.Fill_Rectangle(4, 4, map(curTemp,0,120,4,80), 18);

    
    //draw value
    mylcd.Set_Text_Size(1);
    mylcd.Set_Text_colour(BLUE);
    short offset = (String(curTemp).length()*6);
    mylcd.Print_String(String(curTemp), engineTempPOSX-offset, engineTempPOSY);
    
    //draw circle
    mylcd.Set_Draw_color(BLUE);
    mylcd.Draw_Circle(engineTempPOSX+1,engineTempPOSY,1);
    }
    
  }

  void setExternalTemp()
  {
  Serial.print("temp: ");
    int curTemp;
    curTemp = dht.readTemperature();
    Serial.println(curTemp);
    if(curTemp == lastTemp){
      return;
    }
    lastTemp = curTemp;
    //clear screan
    mylcd.Set_Draw_color(WHITE);
    short offset = (String(curTemp).length()*12);
    mylcd.Fill_Rectangle(externalTempPOSX-offset, externalTempPOSY, externalTempPOSX, externalTempPOSY+13);
    
    //draw value
    mylcd.Set_Text_Size(2);
    mylcd.Set_Text_colour(BLACK);
    mylcd.Set_Text_Back_colour(WHITE);
    mylcd.Print_String(String(curTemp), externalTempPOSX-offset, externalTempPOSY);
    
    //draw circle
    mylcd.Set_Draw_color(BLACK);
    mylcd.Draw_Circle(externalTempPOSX+1,externalTempPOSY+2,2);
  }



void EEPROMWritelong(int address, long value) {
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);
 
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}

String readTotalOdometr()
{
  int addr;
  char tmp;
  String result = "";
  for(addr = 0x70; addr<0x78; addr++){
    tmp = readData(addr);
    result+= String(&tmp);
  }
  return result;
}
String readTripOdometr()
{
  int addr;
  char tmp;
  String result = "";
  for(addr = 0x78; addr<0x7D; addr++){
    tmp = readData(addr);
    result+= String(&tmp);
  }
  return result;
}

// reads a byte of data from i2c memory location addr
byte readData(unsigned int addr)
{
  byte result;
  Wire.beginTransmission(i2caddr);
  // set the pointer position
  //Wire.write((int)(addr >> 8));
  Wire.write((int)(addr & 0xFF));
  Wire.endTransmission(1);
  Wire.requestFrom(i2caddr,1); // get the byte of data
  result = Wire.read();
  return result;
}
