#include <Wire.h>
#include <WiFi.h>
#include <TinyGPS++.h>
#include <ThingSpeak.h>
#include <elapsedMillis.h>
#include <LiquidCrystal_I2C.h>

#define WIFI_SSID "(SSID)"
#define WIFI_PASSWORD "(PASSWORD)"

unsigned long uploadInterval  = 10000;

unsigned long channelNum  = 0000000;
const char* apiKey        = "(Write API Key)";

float latitude, longitude;

LiquidCrystal_I2C lcd (0x27, 16, 2);
TinyGPSPlus gps;
elapsedMillis uploadMillis;
WiFiClient client;

void setup() 
{
  lcd.init();
  lcd.backlight();

  Serial.begin (115200);
  Serial2.begin (9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  
  ThingSpeak.begin(client);
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

  if (uploadMillis >= uploadInterval)
  {
    ThingSpeak.setField (1, latitude);
    ThingSpeak.setField (2, longitude);

    ThingSpeak.writeFields (channelNum, apiKey);

    uploadMillis = 0;
  }
}
