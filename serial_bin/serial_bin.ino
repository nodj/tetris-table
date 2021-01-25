// johan.duparc

#include <Arduino.h>
#include <math.h>

#include "commands.h"
#include "ledstrip.h"


bool ledState = false;
bool ledBlink = true;


void interpretCommand(const CommandInfo& command)
{
	Serial.print("interpretCommand:");
	Serial.println(command.code);

	switch (command.code)
	{
		case CommandCode::LedOn_void:
			ledState = true;
			ledBlink = false;
			break;

		case CommandCode::LedOff_void:
			ledState = false;
			ledBlink = false;
			break;

		case CommandCode::LedBlink_void:
			ledBlink = true;
			break;

		case CommandCode::Fill:
		{
			FillCmd fill(command);
			strip0.Fill(fill.color);
			break;
		}

		default:
			Serial.print("unhanded command ");
			Serial.println(char(command.code));
	}
}

// edit baud rate at runtime: https://forum.arduino.cc/index.php?topic=382040.0
void setup()
{
	// Serial.begin(3000000);
	Serial.begin(9600);
	while (Serial.read() != -1); // flush Rx, just in case
	Serial.flush();
	DeclareSetup(SERIAL_RX_BUFFER_SIZE-1); // allows to reset the c++ part, flush serial buffers, etc...

	pinMode(LED_BUILTIN, OUTPUT);
	Serial.println(R"(==== setup ====)");

	strip0.Setup();
	strip0.Clear();
	Color_24b c = {0, 0, 100};
	strip0.Fill(c);
	strip0.Apply();
	delay(500);
}

void loop()
{
	Serial.println("****");

	ProcessInputSerialStream();
	CommandInfo command;
	if (GetCommandInfo(command))
	{
		interpretCommand(command);
		strip0.Apply(); // as the arduino api buffers the data
	}

	digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
	if (ledBlink) ledState = !ledState;
	delay(50);
}

