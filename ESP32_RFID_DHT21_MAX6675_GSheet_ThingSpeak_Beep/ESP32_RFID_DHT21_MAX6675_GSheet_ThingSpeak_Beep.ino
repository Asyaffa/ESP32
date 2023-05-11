/* 
 NodeMCU 32S
 
 MFRC522
 3.3V = 3V3
 GND  = Common GND
 RST  = 32
 SDA  = 33
 SCK  = 18
 MISO = 19
 MOSI = 23

 DHT21
 VCC  = 5V
 DATA = 4
 GND  = Common GND

 MAX6675
 GND  = Common GND
 VCC  = 5V
 SCK  = 18
 CS   = 5
 SO   = 19

 Relay
 VCC  = External 5V
 IN1  = 25
 IN2  = 26
 GND  = Common GND
*/

#include <SPI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ThingSpeak.h>
#include <elapsedMillis.h>
#include <MFRC522.h>
#include <DHT.h>
#include <max6675.h>

#define DHTPIN  4
#define DHTTYPE DHT21
#define RFIDSDA 33
#define RFIDRST 32

#define WIFI_SSID "(SSID)"
#define WIFI_PASSWORD "(PASSWORD)"

unsigned long channelNum  = 0000000;
const char* apiKey        = "(Write API Key)";

const byte thermoSCK  = 18;
const byte thermoCS   = 5;
const byte thermoSO   = 19;
const byte relayIN1   = 25;  //Cold Side Exhaust
const byte relayIN2   = 26;  //Hot Side Exhaust 

unsigned long readInterval            = 3000;
unsigned long printInterval           = 3000;
unsigned long controlInterval         = 1000;
unsigned long thingspeakSendInterval  = 30000;

const int dhtCSetPointH   = 38;
const int dhtCSetPointL   = 34;
const int dhtCSetPointHs  = 40;
const int dhtCSetPointLs  = 32;

const char* host    = "script.google.com";
const int httpsPort = 443;

const char* ssid        = "-";
const char* password    = "-";
String GOOGLE_SCRIPT_ID = "-";

byte readCard [4];

int dhtCelsius, dhtFahren, dhtHeat, dhtHum;
int thermoCelsius, thermoFahren;
int relayFlag;

String UID                = "";
String sendDHTCelsius     = "";
String sendDHTFahren      = "";
String sendDHTHeat        = "";
String sendDHTHum         = "";
String sendThermoCelsius  = "";
String sendThermoFahren   = "";

String dataSend = "";

bool rfidMode = true;

WiFiClientSecure client;
elapsedMillis readMillis;
elapsedMillis printMillis;
elapsedMillis controlMillis;
elapsedMillis thingspeakSendMillis;
MAX6675 thermocouple (thermoSCK, thermoCS, thermoSO);
MFRC522 mfrc522 (RFIDSDA, RFIDRST);
DHT dht (DHTPIN, DHTTYPE);

void setup()
{
  Serial.begin (115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  delay(1000);

  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init();
  delay(4);
  mfrc522.PCD_DumpVersionToSerial();

  dht.begin();
  ThingSpeak.begin (client);

  pinMode (relayIN1, OUTPUT);
  pinMode (relayIN2, OUTPUT);

  delay(1000);
}

void loop()
{
  if (readMillis >= readInterval) 
  {
    dhtCelsius    = dht.readTemperature();
    dhtFahren     = dht.readTemperature(true);
    dhtHum        = dht.readHumidity();
    dhtHeat       = dht.computeHeatIndex(dhtFahren, dhtHum);
    thermoCelsius = thermocouple.readCelsius();
    thermoFahren  = thermocouple.readFahrenheit();

    sendDHTCelsius    = String (dhtCelsius);
    sendDHTFahren     = String (dhtFahren);
    sendDHTHum        = String (dhtHum);
    sendDHTHeat       = String (dhtHeat);
    sendThermoCelsius = String (thermoCelsius);
    sendThermoFahren  = String (thermoFahren);
    
    readMillis = 0;
  }

  if (controlMillis >= controlInterval)
  {
    if (dhtCelsius >= dhtCSetPointHs || dhtCelsius <= dhtCSetPointLs) //Error
    {
      digitalWrite (relayIN1, LOW);
      digitalWrite (relayIN2, LOW);
      relayFlag = 0;
    }

    else if (dhtCelsius > dhtCSetPointH)  //Cooling Down
    {
      digitalWrite (relayIN1, HIGH);
      digitalWrite (relayIN2, LOW);
      relayFlag = 1;
    }

    else if (dhtCelsius < dhtCSetPointL)  //Heating Up
    {
      digitalWrite (relayIN1, LOW);
      digitalWrite (relayIN2, HIGH);
      relayFlag = 2;
    }
    
    else  //Stable
    {
      digitalWrite (relayIN1, LOW);
      digitalWrite (relayIN2, LOW);
      relayFlag = 3;
    }

    controlMillis = 0;
  }

  if (printMillis >= printInterval)
  {
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

  if (thingspeakSendMillis >= thingspeakSendInterval)
  {
    ThingSpeak.setField (3, dhtCelsius);
    ThingSpeak.setField (4, dhtFahren);
    ThingSpeak.setField (5, dhtHum);
    ThingSpeak.setField (6, dhtHeat);
    ThingSpeak.setField (7, thermoCelsius);
    ThingSpeak.setField (8, thermoFahren);

    ThingSpeak.writeFields (channelNum, apiKey);

    thingspeakSendMillis = 0;
  }

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    // No Card Present
    return;
  }
  
  // Verify if the NUID has been identified
  if (!mfrc522.PICC_ReadCardSerial())
  {
    // No ID Identified
    return;
  }

  rfidRead();

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void rfidRead()
{
  Serial.print(F("\n\nIdentitas Kartu RFID : "));

  for (uint8_t i = 0; i < 4; i++)
  {
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
    UID += String(readCard[i], HEX);
  }

  concatData();
  String data = sendData (dataSend, NULL);
  HandleDataFromGoogle (data);

  Serial.print ("\nAPI : ");
  Serial.println (dataSend);

  UID = "";
}

void concatData()
{
  dataSend =  "uid="        + UID +
              "&Dcelsius="  + sendDHTCelsius +
              "&Dfahren="   + sendDHTFahren +
              "&Dhumi="     + sendDHTHum +
              "&Dheat="     + sendDHTHeat +
              "&Tcelsius="  + sendThermoCelsius +
              "&Tfahren="   + sendThermoFahren;
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

void HandleDataFromGoogle(String data)
{
  int ind               = data.indexOf(":");
  String receiveAcc     = data.substring(0, ind);
  int nextInd           = data.indexOf(":", ind + 1);
  String receiveName    = data.substring(ind + 1, nextInd);
  String receiveStatus  = data.substring(nextInd + 1, data.length());

  if (receiveAcc == "0")
  {
    //Kartu Tidak Terdaftar / Unregistered
  }

  else if (receiveAcc == "1")
  {
    //Kartu Terdaftar / Registered (Asisten)
  }

  delay (3000);
}

/*
================== UID Spreadsheet Google App Script Code ==================
var timeZone = "Asia/Jakarta";
var dateTimeFormat = "dd/MM/yyyy HH:mm:ss";
var logSpreadSheetId = "-";

//Function for RFID REGISTRY
function doGet(e) 
{
  var access = "0";
  var name = 'Unknown';
  var text = 'Unregistered';

  Logger.log(JSON.stringify(e));   // view parameters
  var result = 'OK';      // assume success
  
  if (e.parameter == 'undefined') 
  {
    result = 'No Parameters';
  }
  else 
  {
    var uid = '';
    var dhtCelsius = '';
    var dhtFahren = '';
    var dhtHumi = '';
    var dhtHeat = '';
    var thermoCelsius = '';
    var thermoFahren = '';

    for (var param in e.parameter) 
    {
      var value = stripQuotes(e.parameter[param]);
      switch (param) 
      {
        case 'uid':
          uid = value;
          break;
        case 'Dcelsius':
          dhtCelsius = value;
          break;
        case 'Dfahren':
          dhtFahren = value;
          break;
        case 'Dhumi':
          dhtHumi = value;
          break;
        case 'Dheat':
          dhtHeat = value;
          break;
        case 'Tcelsius':
          thermoCelsius = value;
          break;
        case 'Tfahren':
          thermoFahren = value;
          break;
        default:
          result = "unsupported parameter";
      }
    }
  
    var sheet = SpreadsheetApp.getActive().getActiveSheet();
    var data = sheet.getDataRange().getValues();
    if (data.length == 0)
      return;
    for (var i = 0; i < data.length; i++) 
    {
      if (data[i][0] == uid)
      {
        name = data[i][1];
        access = data[i][2];
        text = data[i][3];
        break;
      } 
    }

    addLog (uid, name, dhtCelsius, dhtFahren, dhtHumi, dhtHeat, thermoCelsius, thermoFahren);      
  }
  
  result = (access + ":" + name + ":" + text );
  return ContentService.createTextOutput(result);
  //return ContentService.createTextOutput(JSON.stringify(json) ).setMimeType(ContentService.MimeType.JSON); 
}

function addLog (logUid, logName, logDCelsius, logDFahren, logDHumi, logDHeat, logTCelsius, logTFahren) 
{   
  var spr = SpreadsheetApp.openById (logSpreadSheetId);
  var sheet = spr.getSheets()[0];
  var data = sheet.getDataRange().getValues();
  var pos = sheet.getLastRow() + 1;
  var rowData = [];

  rowData[0] = Utilities.formatDate (new Date(), timeZone, dateTimeFormat);
  rowData[1] = logUid;
  rowData[2] = logName;
  rowData[3] = logDCelsius;
  rowData[4] = logDFahren;
  rowData[5] = logDHumi;
  rowData[6] = logDHeat;
  rowData[7] = logTCelsius;
  rowData[8] = logTFahren;
  
  var newRange = sheet.getRange(pos, 1, 1, rowData.length);
  newRange.setValues([rowData]);
}

//Remove leading and trailing single or double quotes
function stripQuotes(value) 
{
  return value.replace(/^["']|['"]$/g, "");
}
*/