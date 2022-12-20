#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <HX711_ADC.h>
#include <EEPROM.h>
#include <Servo.h> // include servo library to use its related functions
#define Servo_PWM 6 // A descriptive name for D6 pin of Arduino to provide PWM signal
Servo MG995_Servo;  // Define an instance of of Servo with the name of "MG995_Servo"
  
const int ROW_NUM = 4;    // four rows
const int COLUMN_NUM = 4; // three columns

char keys[ROW_NUM][COLUMN_NUM] = {
    {'*'},
    {'#'}};

byte pin_rows[ROW_NUM] = {5,4};
byte pin_column[COLUMN_NUM] = {3};

// HX711 pin var
const int HX711_dout = 11;
const int HX711_sck = 12;

// Var for Load cell value
const int calVal_eepromAdress = 0;
long t;
int i = 0;

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);
LiquidCrystal_I2C lcd(0x27, 20, 4);
Servo servo;
HX711_ADC LoadCell(HX711_dout, HX711_sck);

void setup()
{
  Serial.begin(9600); // Initialize UART with 9600 Baud rate
  MG995_Servo.attach(Servo_PWM);  // Connect D6 of Arduino with PWM signal pin of servo motor
  lcd.begin();
  lcd.backlight();

  pinMode(HX711_dout, INPUT);
  delay(10);
  Serial.println();
  Serial.println("Starting...");
  LoadCell.begin();

  lcd.setCursor(0, 0);
  lcd.print("Brt: ");
  lcd.setCursor(14, 0);
  lcd.print("gr");

  MG995_Servo.write(0); //Turn clockwise at high speed
  delay(200);
  MG995_Servo.detach();//Stop. You can use deatch function or use write(x), as x is the middle of 0-180 which is 90, but some lack of precision may change this value
  delay(200);
  MG995_Servo.attach(Servo_PWM);

  float calibrationValue;
  calibrationValue = 696.0;
  EEPROM.get(calVal_eepromAdress, calibrationValue);
  long stabilizingtime = 2000;
  boolean _tare = true;
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag())
  {
    Serial.println("Timeout, cek kabel MCU>HX711 pastikan sudah tepat");
    while (1)
      ;
  }
  else
  {
    LoadCell.setCalFactor(calibrationValue);
    Serial.println("Startup selesai");
  }
}

void loop() {
  char key = keypad.getKey();
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0; 
  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;
  i = LoadCell.getData();

  
  if (key == '#')
  {
    Serial.print(key);
          
    Serial.print("Servo kebuka");
    MG995_Servo.write(90); //Turn clockwise at high speed
    delay(200);
    MG995_Servo.detach();//Stop. You can use deatch function or use write(x), as x is the middle of 0-180 which is 90, but some lack of precision may change this value
    delay(200);
    MG995_Servo.attach(Servo_PWM);
  }
  else if(key == '*'){
    MG995_Servo.write(0);
    Serial.print(key);
    i = 0;
    Serial.print("Nilai i: ");
    Serial.println(i);
    tampil(i);
  }
  
  if (newDataReady) {
//    i = LoadCell.getData();
 
    if (millis() > t + serialPrintInterval) {
//      int i = LoadCell.getData();
      
    if(i<0){
      i=0;
    }
      
    if(i >= 900) {
      Serial.print("Servo ketutup");
      MG995_Servo.write(0); //Turn clockwise at high speed
      MG995_Servo.detach();//Stop. You can use deatch function or use write(x), as x is the middle of 0-180 which is 90, but some lack of precision may change this value
      MG995_Servo.attach(Servo_PWM);
      i = 900;
    }
      
    newDataReady = 0;
    t = millis();
    
    Serial.print("Load_cell output val: ");
    Serial.println(i);
    tampil(i);
      
    }
  }

  if(Serial.available() > 0){
    float i;
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay(); //tare
  }

  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tara selesai");
  }
}

int tampil(int j)
{
  lcd.setCursor(4, 0);
  lcd.print("         ");
  
  if (j < 10)
  {
    lcd.setCursor(12, 0);
  }
  else if (j < 100 && j >= 10)
  {
    lcd.setCursor(11, 0);
  }
  else if (j < 1000 && j >= 100)
  {
    lcd.setCursor(10, 0);
  }
  else if (j < 10000 && j >= 1000)
  {
    lcd.setCursor(9, 0);
  }
  else if (j < 100000 && j >= 10000)
  {
    lcd.setCursor(8, 0);
  }
  else if (j < 1000000 && j >= 100000)
  {
    lcd.setCursor(7, 0);
  }
  else if (j < 10000000 && j >= 1000000)
  {
    lcd.setCursor(6, 0);
  }
  else if (j < 100000000 && j >= 10000000)
  {
    lcd.setCursor(5, 0);
  }
  else
  {
    lcd.setCursor(4, 0);
  }
  lcd.print(j);

}
