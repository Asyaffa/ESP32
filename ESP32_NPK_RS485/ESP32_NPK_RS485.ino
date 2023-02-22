//Based on https://www.circuitschools.com/measure-soil-npk-values-using-soil-npk-sensor-with-arduino/

#define RE 32
#define DE 33

const byte nitro[]  = { 0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c };
const byte phos[]   = { 0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc };
const byte kal[]    = { 0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0 };

byte values[11];

void setup() 
{
  Serial.begin(9600);
  Serial2.begin(4800);
  
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);

  // put RS-485 into receive mode
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);

  delay(3000);
}

void loop() 
{
  byte val1, val2, val3;
  val1 = nitrogen();
  delay (250);
  val2 = phosphorous();
  delay (250);
  val3 = potassium();
  delay (250);

  Serial.print("Nitrogen: ");
  val1 = nitrogen();
  Serial.print(" = ");
  Serial.print(val1);
  Serial.println(" mg/kg");
  delay(250);

  Serial.print("Phosphorous: ");
  val2 = phosphorous();
  Serial.print(" = ");
  Serial.print(val2);
  Serial.println(" mg/kg");
  delay(250);

  Serial.print("Potassium: ");
  val3 = potassium();
  Serial.print(" = ");
  Serial.print(val3);
  Serial.println(" mg/kg");
  Serial.println();
  Serial.println();

  delay(3000);
}

byte nitrogen() 
{
  // clear the receive buffer
  Serial2.flush();

  // switch RS-485 to transmit mode
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(1);

  // write out the message
  for (uint8_t i = 0; i < sizeof(nitro); i++) Serial2.write(nitro[i]);

  // wait for the transmission to complete
  Serial2.flush();

  // switching RS485 to receive mode
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);

  // delay to allow response bytes to be received!
  delay(200);

  // read in the received bytes
  for (byte i = 0; i < 7; i++) 
  {
    values[i] = Serial2.read();
  }

  return values[4];
}

byte phosphorous() 
{
  Serial2.flush();

  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);

  for (uint8_t i = 0; i < sizeof(phos); i++) Serial2.write(phos[i]);

  Serial2.flush();
  
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  delay(500);

  for (byte i = 0; i < 7; i++) 
  {
    values[i] = Serial2.read();
  }

  return values[4];
}

byte potassium() 
{
  Serial2.flush();

  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);

  for (uint8_t i = 0; i < sizeof(kal); i++) Serial2.write(kal[i]);

  Serial2.flush();

  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  delay(500);

  for (byte i = 0; i < 7; i++) 
  {
    values[i] = Serial2.read();
  }

  return values[4];
}