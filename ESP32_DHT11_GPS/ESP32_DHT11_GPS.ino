#include <Wire.h>
#include <DHT.h>
#include <TinyGPS++.h>
#include <elapsedMillis.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 26
#define DHTTYPE DHT11

unsigned long readInterval   = 5000;

float h, c, f, hi;
float latitude, longitude;

LiquidCrystal_I2C lcd (0x27, 16, 2);
DHT dht (DHTPIN, DHTTYPE);
TinyGPSPlus gps;
elapsedMillis readMillis;

void setup() 
{
  lcd.init();
  lcd.backlight();

  Serial.begin (115200);
  Serial2.begin (9600);

  dht.begin();
}

void loop() 
{
  while (Serial2.available() > 0)
  {
    gps.encode (Serial2.read());
    if (gps.location.isUpdated())
    {
      latitude = gps.location.lat();
      longitude = gps.location.lng();

      //Serial.print ("Latitude  : ");
      //Serial.print (gps.location.lat(), 4);
      //Serial.print ("\t Longitude : ");
      //Serial.println (gps.location.lng(), 4);

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

  if (readMillis >= readInterval)
  {
    h = dht.readHumidity();
    c = dht.readTemperature();
    f = dht.readTemperature(true);
    hi = dht.computeHeatIndex(f, h);

    Serial.print("Humidity    : "); 
    Serial.print(h);
    Serial.println("%");
    Serial.print("Temperature : "); 
    Serial.print(c);
    Serial.print("*C\t");
    Serial.print(f);
    Serial.println("*F");
    Serial.print("Heat Index  : ");
    Serial.print(hi);
    Serial.println("*F");
    Serial.println("=========================");

    readMillis = 0;
  }
}