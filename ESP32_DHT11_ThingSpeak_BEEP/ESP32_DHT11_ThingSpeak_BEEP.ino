#include <WiFi.h>
#include <DHT.h>
#include <ThingSpeak.h>
#include <elapsedMillis.h>

#define DHTPIN 26
#define DHTTYPE DHT11

#define WIFI_SSID "(SSID)"
#define WIFI_PASSWORD "(PASSWORD)"

unsigned long uploadInterval  = 10000;
unsigned long readInterval    = 5000; 

unsigned long channelNum  = 0000000;
const char* apiKey        = "(Write API Key)";

float h, c, f, hi;

DHT dht (DHTPIN, DHTTYPE);
elapsedMillis uploadMillis;
elapsedMillis readMillis;
WiFiClient client;

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
  
  dht.begin();
  ThingSpeak.begin(client);
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
    ThingSpeak.setField (3, c);
    ThingSpeak.setField (4, f);
    ThingSpeak.setField (5, h);
    ThingSpeak.setField (6, hi);

    ThingSpeak.writeFields (channelNum, apiKey);

    uploadMillis = 0;
  }
}
