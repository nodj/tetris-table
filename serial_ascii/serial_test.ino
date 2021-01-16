#include "Arduino.h"
#include "math.h"

bool ledState = false;
bool ledBlink = true;

enum Command
{
  Command_min = 'a'-1,
  
  Null_void,
  LedBlink_void,
  LedOn_void,
  LedOff_void,
  SayHello_void,
  Repeat_i8,
  
  Command_max
};


void interpretCommand(Command command, int& available)
{
  switch (command)
  {
    case LedOn_void:
      ledState = true;
      ledBlink = false;
      break;

    case LedOff_void:
      ledState = false;
      ledBlink = false;
      break;

    case LedBlink_void:
      ledBlink = true;
      break;
      
    default:
      Serial.print(available);
      Serial.print("unhanded command ");
      Serial.println(char(command));
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  Serial.println("SETUP");
  
  Serial.print("RAMSTART: ");
  Serial.println(RAMSTART);
  Serial.print("RAMEND: ");
  Serial.println(RAMEND);
  Serial.print("RAMEND - RAMSTART: ");
  Serial.println(RAMEND - RAMSTART);
  
}


void loop() {
  int availableChars = Serial.available();
  if (availableChars)
  {
    Serial.print("availableChars: ");
    Serial.println(availableChars);
    int c = -1;
    while (availableChars--)
    {
      c = Serial.read();
      switch(c) 
      {
        case -1:
          Serial.print("wtf");  
          break;

        case '>':
          if (availableChars)
          {
            int c2 = Serial.peek();
            if (c2 > Command_min && c2 < Command_max)
            {
              Serial.read();
              --availableChars;
              
              interpretCommand(Command(c2), availableChars);
              break;
            }
          }
          [[fallthrough]];    
          

        default:
          Serial.print(char(c));  
          break;
      }
    }
    if (c != '\n') Serial.println("");  
  }
  
  digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
  if (ledBlink) ledState = !ledState;
  delay(500);
}
