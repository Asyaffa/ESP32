#include <WiFi.h>
#include <FirebaseESP32.h>
#include <elapsedMillis.h>
#include <DHT.h>

#define DHTPIN 26
#define DHTTYPE DHT11

#define FIREBASE_HOST "(RTDB Reference URL).firebaseio.com"
#define FIREBASE_AUTH "(Database Secrets)"
#define WIFI_SSID "(SSID)"
#define WIFI_PASSWORD "(PASSWORD)"

DHT dht (DHTPIN, DHTTYPE);
elapsedMillis uploadMillis;
elapsedMillis readMillis;
FirebaseData firebaseData;
FirebaseJson json;

unsigned long uploadInterval = 8000;
unsigned long readInterval = 3000;

float h, c, f, hi;

void setup() 
{
  Serial.begin (115200);
  
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
    Firebase.setFloat (firebaseData, "/DHT11/y_celsius", c);
    Firebase.setFloat (firebaseData, "/DHT11/y_fahrenheit", f);
    Firebase.setFloat (firebaseData, "/DHT11/y_heat_index", hi);
    Firebase.setFloat (firebaseData, "/DHT11/y_humidity", h);
    
    uploadMillis = 0;
  }
}

