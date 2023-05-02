/* 
 NodeMCU 32S
 
 GPS
 VCC  = 5V
 RX   = 17
 TX   = 16
 GND  = Common GND
 
 MAX6675
 GND  = Common GND
 VCC  = 3V3
 SCK  = 18
 CS   = 5
 SO   = 19
 
 DHT21
 VCC  = 3V3
 DATA = 4
 GND  = Common GND
 
 Relay
 VCC  = External 5V
 IN1  = 25
 IN2  = 26
 GND  = Common GND
*/

#include <WiFi.h>
#include <ThingSpeak.h>
#include <elapsedMillis.h>
#include <DHT.h>
#include <max6675.h>
#include <TinyGPS++.h>

#define WIFI_SSID     "(SSID)"
#define WIFI_PASSWORD "(PASSWORD)"

#define DHTPIN  4
#define DHTTYPE DHT21

const byte thermoSCK  = 18;
const byte thermoCS   = 5;
const byte thermoSO   = 19;
const byte relayIN1   = 25;  //Cold Peltier
const byte relayIN2   = 26;  //Hot Peltier

unsigned long readInterval    = 2000;
unsigned long printInterval   = 3000;
unsigned long controlInterval = 1000;
unsigned long uploadInterval  = 10000;

unsigned long channelNum  = 0000000;
const char* apiKey        = "(Write API Key)";

const int dhtCSetPointH   = 38;
const int dhtCSetPointL   = 34;
const int dhtCSetPointHs  = 40;
const int dhtCSetPointLs  = 32;

float gpsLat, gpsLong;
int thermoCelsius, thermoFahren;
int dhtCelsius, dhtFahren, dhtHeat, dhtHum;
int relayFlag;

elapsedMillis readMillis;
elapsedMillis printMillis;
elapsedMillis controlMillis;
elapsedMillis uploadMillis;
DHT dht (DHTPIN, DHTTYPE);
MAX6675 thermocouple (thermoSCK, thermoCS, thermoSO);
TinyGPSPlus gps;
WiFiClient client;

void setup()
{
  Serial.begin (115200);
  Serial2.begin (9600);

  WiFi.begin (WIFI_SSID, WIFI_PASSWORD);
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

  pinMode (relayIN1, OUTPUT);
  pinMode (relayIN2, OUTPUT);

  delay (1000);
}

void loop()
{
  while (Serial2.available() > 0)
  {
    gps.encode (Serial2.read());
    if (gps.location.isUpdated())
    {
      gpsLat  = gps.location.lat();
      gpsLong = gps.location.lng();
    }
  }

  if (readMillis >= readInterval)
  {
    thermoCelsius = thermocouple.readCelsius();
    thermoFahren  = thermocouple.readFahrenheit();
    dhtCelsius    = dht.readTemperature();
    dhtFahren     = dht.readTemperature(true);
    dhtHum        = dht.readHumidity();
    dhtHeat       = dht.computeHeatIndex(dhtFahren, dhtHum);
    
    readMillis = 0;
  }

  if (controlMillis >= controlInterval)
  {
    if (dhtCelsius >= dhtCSetPointHs || dhtCelsius <= dhtCSetPointLs)
    {
      digitalWrite (relayIN1, LOW);
      digitalWrite (relayIN2, LOW);
      relayFlag = 0;
    }

    else if (dhtCelsius > dhtCSetPointH)
    {
      digitalWrite (relayIN1, HIGH);
      digitalWrite (relayIN2, LOW);
      relayFlag = 1;
    }

    else if (dhtCelsius < dhtCSetPointL)
    {
      digitalWrite (relayIN1, LOW);
      digitalWrite (relayIN2, HIGH);
      relayFlag = 2;
    }
    
    else
    {
      digitalWrite (relayIN1, LOW);
      digitalWrite (relayIN2, LOW);
      relayFlag = 3;
    }

    controlMillis = 0;
  }

  if (printMillis >= printInterval)
  {
    Serial.print ("Latitude       : ");
    Serial.print (gpsLat);
    Serial.print ("\t Longitude      : ");
    Serial.println (gpsLong);
    Serial.print ("Thermocouple C : ");
    Serial.print (thermoCelsius);
    Serial.print ("*C");
    Serial.print ("\t Thermocouple F : ");
    Serial.print (thermoFahren);
    Serial.println ("*F");
    Serial.print ("DHT21 C        : "); 
    Serial.print (dhtCelsius);
    Serial.print ("*C");
    Serial.print ("\t DHT21 F        : ");
    Serial.print (dhtFahren);
    Serial.println ("*F");
    Serial.print ("Humidity       : "); 
    Serial.print (dhtHum);
    Serial.print ("%");
    Serial.print ("\t Heat Index     : ");
    Serial.println (dhtHeat);
    Serial.println ("==================================================");
    
    Serial.print ("Peltier Condition : ");
    switch (relayFlag)
    {
      case (0):
        Serial.println ("Error...");
        break;
      case (1):
        Serial.println ("Cooling...");
        break;
      case (2):
        Serial.println ("Heating...");
        break;
      case (3):
        Serial.println ("Stable...");
        break;
    }
    Serial.println ("==================================================");

    printMillis = 0;
  }

  if (uploadMillis >= uploadInterval)
  {
    ThingSpeak.setField (1, gpsLat);
    ThingSpeak.setField (2, gpsLong);
    ThingSpeak.setField (3, thermoCelsius);
    ThingSpeak.setField (4, thermoFahren);
    ThingSpeak.setField (5, dhtCelsius);
    ThingSpeak.setField (6, dhtFahren);
    ThingSpeak.setField (7, dhtHum);
    ThingSpeak.setField (8, dhtHeat);

    ThingSpeak.writeFields (channelNum, apiKey);    
        
    uploadMillis = 0;
  }
}
