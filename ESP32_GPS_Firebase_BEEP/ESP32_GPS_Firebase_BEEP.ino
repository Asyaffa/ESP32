//Based On : https://github.com/ahmadlogs/nodemcu-arduino-ide/blob/main/nodemcu-gps-firebase-mit/nodemcu-gps-firebase-mit.ino

#include <Wire.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <elapsedMillis.h>
#include <TinyGPS++.h>
#include <LiquidCrystal_I2C.h>

#define FIREBASE_HOST "(RTDB Reference URL).firebaseio.com"
#define FIREBASE_AUTH "(Database Secrets)"
#define WIFI_SSID "(SSID)"
#define WIFI_PASSWORD "(PASSWORD)"

LiquidCrystal_I2C lcd (0x27, 16, 2);
elapsedMillis uploadMillis;
FirebaseData firebaseData;
FirebaseJson json;
TinyGPSPlus gps;

unsigned long uploadInterval = 5000;

float latitude = 0;
float longitude = 0;

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

  Firebase.begin (FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi (true);
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
    if (Firebase.setFloat (firebaseData, "/GPS/x_latitude", latitude))
    {
      print_ok();
    }
    else
    {
      print_fail();
    }

    if (Firebase.setFloat (firebaseData, "/GPS/x_longitude", longitude))
    {
      print_ok();
    }
    else
    {
      print_fail();
    }

    uploadMillis = 0;
  }
}

void print_ok()
{
  Serial.println("------------------------------------");
  Serial.println("OK");
  Serial.println("PATH: " + firebaseData.dataPath());
  Serial.println("TYPE: " + firebaseData.dataType());
  Serial.println("ETag: " + firebaseData.ETag());
  Serial.println("------------------------------------");
  Serial.println();
}

void print_fail()
{
  Serial.println("------------------------------------");
  Serial.println("FAILED");
  Serial.println("REASON: " + firebaseData.errorReason());
  Serial.println("------------------------------------");
  Serial.println();
}