#include <Wire.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <elapsedMillis.h>
#include <DHT.h>
#include <TinyGPS++.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 26
#define DHTTYPE DHT11

#define FIREBASE_HOST "(RTDB Reference URL).firebaseio.com"
#define FIREBASE_AUTH "(Database Secrets)"
#define WIFI_SSID "(SSID)"
#define WIFI_PASSWORD "(PASSWORD)"

LiquidCrystal_I2C lcd (0x27, 16, 2);
DHT dht (DHTPIN, DHTTYPE);
elapsedMillis uploadMillis;
elapsedMillis readMillis;
FirebaseData firebaseData;
FirebaseJson json;
TinyGPSPlus gps;

unsigned long uploadInterval = 8000;
unsigned long readInterval = 3000;

float h, c, f, hi;
float latitude, longitude;

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

  if (uploadMillis >= uploadInterval)
  {
    Firebase.setFloat (firebaseData, "/GPS/x_latitude", latitude);
    Firebase.setFloat (firebaseData, "/GPS/x_longitude", longitude);

    Firebase.setFloat (firebaseData, "/DHT11/y_celsius", c);
    Firebase.setFloat (firebaseData, "/DHT11/y_fahrenheit", f);
    Firebase.setFloat (firebaseData, "/DHT11/y_heat_index", hi);
    Firebase.setFloat (firebaseData, "/DHT11/y_humidity", h);
    
    uploadMillis = 0;
  }
}