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
 VCC  = 3V3
 DATA = 4
 GND  = Common GND
*/

#include <SPI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <elapsedMillis.h>
#include <MFRC522.h>
#include <DHT.h>

#define DHTPIN  4
#define DHTTYPE DHT21
#define RFIDSDA 33
#define RFIDRST 32

unsigned long readInterval = 2000;

const char* host    = "script.google.com";
const int httpsPort = 443;

const char* ssid        = "-";
const char* password    = "-";
String GOOGLE_SCRIPT_ID = "-";

byte readCard [4];

int dhtCelsius, dhtFahren, dhtHeat, dhtHum;

String UID          = "";
String sendCelsius  = "";
String sendFahren   = "";
String sendHeat     = "";
String sendHum      = "";

String dataSend = "";

bool rfidMode = true;

WiFiClientSecure client;
elapsedMillis readMillis;
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
  //Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  dht.begin();

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

    sendCelsius   = String (dhtCelsius);
    sendFahren    = String (dhtFahren);
    sendHum       = String (dhtHum);
    sendHeat      = String (dhtHeat);
    
    readMillis = 0;
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

  Serial.print ("\n");
  Serial.print ("DATA : ");
  Serial.println (dataSend);

  UID = "";
}

void concatData()
{
  dataSend =  "uid="      + UID +
              "&celsius=" + sendCelsius +
              "&fahren="  + sendFahren +
              "&humi="    + sendHum +
              "&heat="    + sendHeat;
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

    for (var param in e.parameter) 
    {
      var value = stripQuotes(e.parameter[param]);
      switch (param) 
      {
        case 'uid':
          uid = value;
          break;
        case 'celsius':
          dhtCelsius = value;
          break;
        case 'fahren':
          dhtFahren = value;
          break;
        case 'humi':
          dhtHumi = value;
          break;
        case 'heat':
          dhtHeat = value;
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

    addLog (uid, name, dhtCelsius, dhtFahren, dhtHumi, dhtHeat);      
  }
  
  result = (access + ":" + name + ":" + text );
  return ContentService.createTextOutput(result);
  //return ContentService.createTextOutput(JSON.stringify(json) ).setMimeType(ContentService.MimeType.JSON); 
}

function addLog (logUid, logName, logCelsius, logFahren, logHumi, logHeat) 
{   
  var spr = SpreadsheetApp.openById (logSpreadSheetId);
  var sheet = spr.getSheets()[0];
  var data = sheet.getDataRange().getValues();
  var pos = sheet.getLastRow() + 1;
  var rowData = [];

  rowData[0] = Utilities.formatDate (new Date(), timeZone, dateTimeFormat);
  rowData[1] = logUid;
  rowData[2] = logName;
  rowData[3] = logCelsius;
  rowData[4] = logFahren;
  rowData[5] = logHumi;
  rowData[6] = logHeat;
  
  var newRange = sheet.getRange(pos, 1, 1, rowData.length);
  newRange.setValues([rowData]);
}

//Remove leading and trailing single or double quotes
function stripQuotes(value) 
{
  return value.replace(/^["']|['"]$/g, "");
}
*/
