/* 
 WeMos LoLin 32
 RX2 = 16
 TX2 = 17
 */

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>

LiquidCrystal_I2C lcd (0x27, 16, 2);

TinyGPSPlus gps;

void setup()
{
  lcd.init();
  lcd.backlight();

  Serial.begin (115200);
  Serial2.begin (9600);
}

void loop()
{
  while (Serial2.available() > 0)
  {
    gps.encode (Serial2.read());
    if (gps.location.isUpdated())
    {
      Serial.print ("Latitude  : ");
      Serial.print (gps.location.lat(), 4);
      Serial.print ("\t Longitude : ");
      Serial.println (gps.location.lng(), 4);

      lcd.setCursor (0, 0);
      lcd.print ("            ");
      lcd.setCursor (0, 1);
      lcd.print ("            ");

      lcd.setCursor (0, 0);
      lcd.print ("LT: ");
      lcd.print (gps.location.lat(), 6);
      lcd.setCursor (0, 1);
      lcd.print ("LG: ");
      lcd.print (gps.location.lng(), 6);
    }
  }
}