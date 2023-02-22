#include <DHT.h>

#define DHTPIN 26
#define DHTTYPE DHT11

DHT dht (DHTPIN, DHTTYPE);

void setup() 
{
  Serial.begin (115200);
  
  dht.begin();
}

void loop() 
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  float hi = dht.computeHeatIndex(f, h);

  Serial.print("Humidity    : "); 
  Serial.print(h);
  Serial.println("%");
  Serial.print("Temperature : "); 
  Serial.print(t);
  Serial.print("*C\t");
  Serial.print(f);
  Serial.println("*F");
  Serial.print("Heat Index  : ");
  Serial.print(hi);
  Serial.println("*F");
  Serial.println("=========================");

  delay (2000);
}
