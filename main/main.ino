/*
 * Board: Maple mini form STM32F1 Boards (STM32Duino.com)
 * Use libusb-win32 driver for Maple 003/Maple DFU Device(bootloader mode)
 * maple_mini_boot20.bin bootloader 2.0
 * 
 * LCD SSD1283A
 * LED -> PB6 -> Maple mini pin 16
 * SCK -> PA5 -> Maple mini pin 6
 * SDA -> PA7 -> Maple mini pin 4
 * A0  -> PB7 -> Maple mini pin 15
 * RST -> PA1 -> Maple mini pin 10 
 * CS  -> PA4 -> Maple mini pin 7
 * 
 * DHT22
 * Data pin -> PA8 -> Maple mini pin 27
 * 
 * Temperature -> PA2 -> Maple mini pin 9 
 * 
 * KEY -> PB12 -> Maple mini pin 31
 * LIGHTS -> PB13 -> Maple mini pin 30
 * BEEPER -> PB14 -> Maple mini pin 29
 * RPM -> PA0 -> Maple mini pin 11
 * BUTTON -> PA15 -> Maple mini pin 20
 */

#include "DHT.h"
#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_SPI.h> //Hardware-specific library
#include <EEPROM.h>
#include <RTClock.h>
#include <time.h>
#include <Wire.h>    // for I2C

#define i2caddr 0x50
#define STM_I2C2_FREQ 400000
TwoWire Wire2 (2,I2C_FAST_MODE);
#define Wire Wire2

#define DHTPIN PA8
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

RTClock rtc(RTCSEL_LSI);
time_t tt;
tm_t tm, newtm;
String blinkChar = ":";

#define LED_PIN PB1
#define LIGHTS PB13
#define KEY PB12
#define BEEPER PB14
#define RPM PA0
#define TEMP PA2
#define BUTTON PA15

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
int16_t odometrPOSY = 78;
int16_t tripPOSX = 116;
int16_t tripPOSY = 98;

int16_t lastEngineTemp;
int16_t lastTemp;
int16_t lastGear;
int16_t rpm;
int lastRPM;
String lastTrip;
String lastOdometr;
int lastTime;
int lastSecond;
int curTemp;
String lastDate;
unsigned long uptime;
unsigned long uptime2;
unsigned long uptime3;
unsigned long uptime4;
unsigned long timeKey;
bool alarm = 0;
bool tempAlarm = 0;
bool invertLCD = 0;
bool state = 0;
String curDate;
String day;
String month;
int hour;
int minute;
int second;

long buttonTimer = 0;
long longPressTime = 2000;
long superLongPressTime = 5000;

boolean buttonActive = false;
boolean longPressActive = false;
boolean superLongPressActive = false;
bool backlidState = 1;


float i = -20;
float g = 0;


volatile int pwm_value = 0;
volatile int prev_time = 0;

//LCDWIKI_SPI mylcd(SSD1283A,10,9,8,-1); //Pro Mini
//LCDWIKI_SPI mylcd(SSD1283A,15,16,4,-1); //ESP8266

LCDWIKI_SPI mylcd(SSD1283A,PA4,PB7,PA1,PB6); //Maple mini //hardware spi,cs,cd,reset,led
//LCDWIKI_SPI mylcd(SSD1283A,PA4,PB7,PA6,PA7,PA1,PA5,-1);//software spi,model,cs,cd,miso,mosi,reset,clk,led

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
  delay(2000);
  Serial.begin(9600);
  delay(200);
  Serial.println("Pin modes");
  pinMode(LED_PIN, OUTPUT);
  pinMode(KEY, INPUT_PULLDOWN);
  pinMode(LIGHTS, INPUT_PULLDOWN);
  pinMode(RPM, INPUT_PULLDOWN);
  pinMode(TEMP, INPUT_ANALOG);
  pinMode(BUTTON , INPUT_PULLUP);
  pinMode(BEEPER, OUTPUT);
  
  Serial.println("I2C start");
  Wire.begin();         // wake up the I2C
  Serial.println("DHT start");
  dht.begin();
  Serial.println("LCD start");
  mylcd.Init_LCD();
  mylcd.Set_Rotation(1);
  mylcd.Set_Text_Mode(1);
  mylcd.Fill_Screen(WHITE);
  animation();
  initTemp();
  Serial.println("Time start");
  lastTime = 99;
  //setTime(2020,01,01,00,00,00);
  updateTime();
  updateDate();
  //initGear();
  
  Serial.println("Interrupts start");
  attachInterrupt(RPM, rising, FALLING);
  rtc.attachSecondsInterrupt(countRPM);
  
  state = true;
}

void reinit(){

  if(digitalRead(LIGHTS)){
    if(!state){
  Serial.print("Test");
      Serial.println("Ignition");
      Serial.println("Rest variables");
      lastOdometr = "";
      lastTrip = "";
      curTemp = 0;
      lastTime = 99999;
      lastDate = "";
      //Wire.end();         // wake up the I2C
      //dht.end();
      Serial.println("Wire restart");
      Wire.begin();         // wake up the I2C
      
      Serial.println("DHT restart");
      dht.begin();

      Serial.println("LCD restart");
      mylcd.Init_LCD();
      mylcd.Set_Rotation(1);
      mylcd.Set_Text_Mode(1);
      mylcd.Fill_Screen(WHITE);
      
      Serial.println("Welcome to Varadero LCD");
      animation();
      Serial.println("Init temp");
      initTemp();
      state = true;
    }
  }
  else {
    state = false;
    bye();
    Serial.println("Bye");
  }
}




void loop() {
  alarm = keyOn();
  reinit();
  if(millis() - uptime >=500 && digitalRead(LIGHTS)){
  //if(millis() - uptime >=500 && digitalRead(KEY)){
    uptime = millis();
    blink();
    rtc.getTime(tm);
    updateTime();
    updateDate();
    setEngineTemp(i);
    setExternalTemp();
    setOdometr(readTotalOdometr());
    setTrip(readTripOdometr());
    //mylcd.Invert_Display(1);
    Serial.print("Temp: ");
    Serial.println(analogRead(TEMP));
    Serial.print("RPM: ");
    Serial.println(lastRPM*60);
  i+=5;
  if (i==120) i=0;
  }
  rpmCount();
  beep();
  button();
  
//    //EEPROMWritelong(EEPROM_TRIP, trip);
//    setGear(g);
//    g++;
//    if (g==7) g=0;
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
void bye()
{
  mylcd.Set_Text_colour(BLACK);
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("Bye", 20, GEAR_POS_Y+GEAR_SIZE_Y-18);
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
  
void updateDate()
{
  day = String(tm.day);
  month = String(tm.month);
  if(tm.day<10) day = "0"+String(tm.day);
  if(tm.month<10) month = "0"+String(tm.month);
  curDate = day + "/" + month + "/" + String(1970+tm.year);
    if(lastDate != curDate){
       lastDate = curDate;
    mylcd.Set_Draw_color(WHITE);
    mylcd.Fill_Rectangle(dateTimePOSX-10, dateTimePOSY, dateTimePOSX+120, dateTimePOSY+14);
    mylcd.Set_Text_colour(BLACK);
    mylcd.Set_Text_Size(2);
    mylcd.Print_String(String(curDate), dateTimePOSX-10, dateTimePOSY);
    }
  }
  
void updateTime()
{
  hour = tm.hour;
  minute = tm.minute;
  second = tm.second;
  if(lastTime != hour+minute){
    lastTime = hour+minute;        
    mylcd.Set_Draw_color(WHITE);
    mylcd.Fill_Rectangle(dateTimePOSX, dateTimePOSY+20, dateTimePOSX+43, dateTimePOSY+48);
    mylcd.Set_Draw_color(WHITE);
    mylcd.Fill_Rectangle(dateTimePOSX+54, dateTimePOSY+20, dateTimePOSX+98, dateTimePOSY+48);
    mylcd.Set_Text_colour(BLACK);
    mylcd.Set_Text_Size(4);
    if (hour < 10 ){
      mylcd.Print_String("0"+String(hour), dateTimePOSX, dateTimePOSY+20);
    }
    else{
      mylcd.Print_String(String(hour), dateTimePOSX, dateTimePOSY+20);
    }
    if (minute < 10 ){
    mylcd.Print_String("0"+String(minute), dateTimePOSX+55, dateTimePOSY+20);
    }
    else{
    mylcd.Print_String(String(minute), dateTimePOSX+55, dateTimePOSY+20);
    }
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
      if(invertLCD) mylcd.Set_Draw_color(BLUE);
      tempAlarm = 1;
    }
    else{
      tempAlarm = 0;
      mylcd.Set_Draw_color(BLACK);
    }
    mylcd.Fill_Rectangle(4, 4, map(curTemp,0,120,4,80), 18);

    
    //draw value
    mylcd.Set_Text_Size(1);
    mylcd.Set_Text_colour(BLUE);
    if(invertLCD) mylcd.Set_Text_colour(MAGENTA);
    short offset = (String(curTemp).length()*6);
    mylcd.Print_String(String(curTemp), engineTempPOSX-offset, engineTempPOSY);
    
    //draw circle
    mylcd.Set_Draw_color(BLUE);
    if(invertLCD) mylcd.Set_Draw_color(MAGENTA);
    mylcd.Draw_Circle(engineTempPOSX+1,engineTempPOSY,1);
    }
    
}

  void setExternalTemp()
  {
    curTemp = dht.readTemperature();
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

void blink ()
{
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  if (blinkChar.equals(String(":"))){
    blinkChar = "";
    if(alarm){
      invertLCD = 0;
    }
  }
  else{
    blinkChar = ":";
    if(alarm){
      invertLCD = 1;
    }
  }
  if(alarm){
    mylcd.Invert_Display(invertLCD);
  }
  else {
    mylcd.Invert_Display(0);
  }
  
  mylcd.Set_Draw_color(WHITE);
  mylcd.Set_Text_colour(BLACK);
  mylcd.Set_Text_Size(4);
  mylcd.Fill_Rectangle(dateTimePOSX+46, dateTimePOSY+24, dateTimePOSX+50, dateTimePOSY+44);
  mylcd.Print_String(blinkChar, dateTimePOSX+39, dateTimePOSY+20);

  
}
void beep()
{
  if(alarm){
    if(millis() - uptime2 >=200){
      uptime2 = millis();
      digitalWrite(BEEPER, !digitalRead(BEEPER));
    }
  }
  else{
    digitalWrite(BEEPER, LOW);
  }
  
}

int rpmCount(){
  //return digitalRead(RPM);
  //return pwm_value;
  return lastRPM*60;
  //return pulseIn(RPM, HIGH);

}
void button(){

//  if (!digitalRead(BUTTON)) {
//    if (buttonActive == false) {
//      buttonTimer = millis();
//      buttonActive = true;
//    }
//  }
//  else{
//    if (buttonActive == true) {
//      if(millis() - buttonTimer < 500){
//        Serial.println("button -> short");
//      }
//      else if(millis() - buttonTimer < 5000 && millis() - buttonTimer > 2000){
//        Serial.println("button -> long");
//      }else if(millis() - buttonTimer > 5000){
//        backlidState = !backlidState;
//        backlidControl(backlidState);
//      }
//      buttonTimer = 0;
//      buttonActive = false;
//    }
//  }
//
  if (digitalRead(BUTTON) == LOW) {
    if (buttonActive == false) {
      buttonActive = true;
      buttonTimer = millis();
    }
    if ((millis() - buttonTimer > longPressTime) && (longPressActive == false)) {
      longPressActive = true;
      Serial.println("button -> long");
    }
    if ((millis() - buttonTimer > superLongPressTime) && (superLongPressActive == false)) {
      superLongPressActive = true;
      backlidState = !backlidState;
      backlidControl(backlidState);
      Serial.println("toggle backlight");
    }
  }
  else{
    if (buttonActive == true) {
      if (longPressActive == true) {
        longPressActive = false;
      }
      if (superLongPressActive == true) {
        superLongPressActive = false;
      }
      else {
        Serial.println("button -> short");
      }
      buttonActive = false;
    }
  }
}

void backlidControl( bool i)
{
  mylcd.Led_control(i);
  backlidState = i;
}

bool keyOn ()
{
  if(digitalRead(LIGHTS) && rpmCount() < 500){
    if(millis() - timeKey > 60*1000){
      return true;
    }
    //timeKey = millis();
  }
  if(rpmCount() > 500){
    timeKey = millis();
  }
  return false;
}
void setTime(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss)
{
  newtm.year = YYYY - 1970;
  newtm.month = MM;
  newtm.day = DD;
  newtm.hour = hh;
  newtm.minute = mm;
  newtm.second = ss;
  Serial.println("Time start2");
  rtc.setTime(rtc.makeTime(newtm));
  Serial.println("Time start3");
}

void rising() {

  static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (interrupt_time - last_interrupt_time > 2)
 {
   rpm++; 
 }
 last_interrupt_time = interrupt_time;
 
  //attachInterrupt(RPM, falling, FALLING);
  //prev_time = micros();
//  if(micros()-prev_time > 100){
//    prev_time = micros();
//    
//  }
}
 
//void falling() {
//  attachInterrupt(RPM, rising, RISING);
//  pwm_value = micros()-prev_time;
//  //Serial.println(pwm_value);
//}

void countRPM(){
  lastRPM = rpm;
  rpm=0;
}
