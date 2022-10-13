#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <HX711_ADC.h>
#include <EEPROM.h>

const int ROW_NUM = 4;    // four rows
const int COLUMN_NUM = 4; // three columns

String stringAngka;

bool isKeyActive = false;
char keys[ROW_NUM][COLUMN_NUM] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

byte pin_rows[ROW_NUM] = {9, 8, 7, 6};
byte pin_column[COLUMN_NUM] = {5, 4, 3, 2};

// HX711 pin var
const int HX711_dout = 11;
const int HX711_sck = 12;

// Var for Load cell value
const int calVal_eepromAdress = 0;
long t;

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);
LiquidCrystal_I2C lcd(0x27, 20, 4);
Servo servo;
HX711_ADC LoadCell(HX711_dout, HX711_sck);

void setup()
{
  Serial.begin(9600);
  servo.attach(10);
  servo.write(0);
  lcd.begin();
  lcd.backlight();

  pinMode(HX711_dout, INPUT);
  delay(10);
  Serial.println();
  Serial.println("Starting...");
  LoadCell.begin();

  lcd.setCursor(0, 0);
  lcd.print("Berat: ");
  lcd.setCursor(14, 0);
  lcd.print("gr");

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

void loop()
{
  char key = keypad.getKey();

  //  Start looping for load cell
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0;

  // check for new data/start next conversion:
  if (LoadCell.update())
    newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady)
  {
    if (millis() > t + serialPrintInterval)
    {
      int i = LoadCell.getData();
      Serial.print("Load_cell output val: ");
      Serial.println(i);
      if (i < 0)
      {
        i = 0;
      }

      // Keypad condition
      if (key)
      {
        if (key == '*')
        {
          isKeyActive = true;
        }
        else if (key == '#')
        {
          isKeyActive = false;
          servo.write(90);
        }
        else if (key == 'C')
        {
          stringAngka = "";
          passDisplay(stringAngka);
        }
        else if (isKeyActive)
        {
          stringAngka += key;
          passDisplay(stringAngka);
        }
      }

      if (i >= stringAngka)
      {
        servo.write(0);
      }
      
      // Show weight
      tampil(i);
      newDataReady = 0;
      t = millis();

      
    }
  }
  if (Serial.available() > 0)
  {
    float i;
    char inByte = Serial.read();
    if (inByte == 't')
      LoadCell.tareNoDelay(); // tare
  }

  // check if last tare operation is complete
  if (LoadCell.getTareStatus() == true)
  {
    Serial.println("Tara selesai");
  }

}

void passDisplay(String password)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Berat: ");
  lcd.print(password);
}

void statusDisplay(String status)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(status);
}

void tampil(int j)
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
