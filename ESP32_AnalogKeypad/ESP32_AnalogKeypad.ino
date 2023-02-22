int analogInput = 0;
int analogMap;
char keypadInput;
const byte keypadPin = 4;

void setup() 
{
  Serial.begin (9600);
  pinMode (keypadPin, INPUT_PULLUP);
}

void loop() 
{
  analogInput = analogRead (keypadPin);
  analogMap = map (analogInput, 0, 4095, 0, 255);
  
  if (analogMap > 82 && analogMap < 86)
  {
    keypadInput = '0'; 
  }
  else if (analogMap > 253 && analogMap < 256)
  {
    keypadInput = '1'; 
  }
  else if (analogMap > 225 && analogMap < 229)
  {
    keypadInput = '2'; 
  }
  else if (analogMap > 194 && analogMap < 198)
  {
    keypadInput = '3'; 
  }
  else if (analogMap > 149 && analogMap < 153)
  {
    keypadInput = '4'; 
  }
  else if (analogMap > 139 && analogMap < 143)
  {
    keypadInput = '5'; 
  }
  else if (analogMap > 131 && analogMap < 135)
  {
    keypadInput = '6'; 
  }
  else if (analogMap > 109 && analogMap < 113)
  {
    keypadInput = '7'; 
  }
  else if (analogMap > 103 && analogMap < 107)
  {
    keypadInput = '8'; 
  }
  else if (analogMap > 98 && analogMap < 102)
  {
    keypadInput = '9'; 
  }
  else if (analogMap > 86 && analogMap < 90)
  {
    keypadInput = '*'; 
  }
  else if (analogMap > 79 && analogMap < 83)
  {
    keypadInput = '#'; 
  }
  else if (analogMap > 175 && analogMap < 179)
  {
    keypadInput = 'A'; 
  }
  else if (analogMap > 122 && analogMap < 126)
  {
    keypadInput = 'B'; 
  }
  else if (analogMap > 93 && analogMap < 97)
  {
    keypadInput = 'C'; 
  }
  else if (analogMap > 76 && analogMap < 80)
  {
    keypadInput = 'D'; 
  }
  
  Serial.print ("ANALOG VALUE : ");
  Serial.print (analogMap);
  Serial.print ("\t KEYPAD VALUE : ");
  Serial.println (keypadInput);

  keypadInput = ' ';
  
  delay (500);
}