#include <DHT.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <elapsedMillis.h>

#define DHTPIN 26
#define DHTTYPE DHT11

#define WIFI_SSDI "UGMURO-1"
#define WIFI_PASSWORD "Piscok2000"
#define GOOGLE_SCRIPT_ID "XXX"  

DHT dht (DHTPIN, DHTTYPE);
elapsedMillis readMillis;
elapsedMillis uploadMillis;
WiFiClientSecure client;

const char* host = "script.google.com";
const int httpsPort = 443;

unsigned long uploadInterval = 8000;
unsigned long readInterval = 3000;

float h, c, f, hi;
String data;
String dataInkubator = "";

DHT dht (DHTPIN, DHTTYPE);

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
    
    
    uploadMillis = 0;
  }  
}

void concatData()
{
  dataInkubator = "humidity=" + h +
                  "&celsius=" + c +
                  "&fahrenheit=" + f +
                  "&heatindex=" + hi;
}

String sendData(String params, char* domain)
{
  //google scripts requires two get requests
  bool needRedir = false;
  if (domain == NULL)
  {
    domain = (char*)host;
    needRedir = true;
    params = "/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?" + params;
  }

  //Serial.println(*domain);
  String result = "";
  client.setInsecure();

  //Serial.print("connecting to ");
  //Serial.println(host);
  if (!client.connect(host, httpsPort))
  {
    //Serial.println("Connection Failed");
    return "";
  }

  //Serial.print("requesting URL: ");
  //Serial.println(params);
  client.print(String("GET ") + params + " HTTP/1.1\r\n" +
               "Host: " + domain + "\r\n" +
               "Connection: close\r\n\r\n");

  //Serial.println("request sent");
  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    //Serial.println(line);
    if (needRedir)
    {
      int ind = line.indexOf("/macros/echo?user");
      if (ind > 0)
      {
        //Serial.println(line);
        line = line.substring(ind);
        ind = line.lastIndexOf("\r");
        line = line.substring(0, ind);
        //Serial.println(line);
        result = line;
      }
    }

    if (line == "\r")
    {
      //Serial.println("headers received");
      break;
    }
  }

  while (client.available())
  {
    String line = client.readStringUntil('\n');
    if (!needRedir)
      if (line.length() > 5)
        result = line;
    //Serial.println(line);
  }

  if (needRedir)
    return sendData(result, "script.googleusercontent.com");

  else
    return result;
}