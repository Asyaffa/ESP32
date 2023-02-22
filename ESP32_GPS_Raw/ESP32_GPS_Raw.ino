void setup() 
{
  Serial.begin (115200);
  Serial2.begin (9600);
}

void loop() {
  while (Serial2.available() > 0) 
  {
    byte gpsData = Serial2.read();
    Serial.write(gpsData);
  }
}