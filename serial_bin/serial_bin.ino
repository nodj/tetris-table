// johan.duparc

#include <Arduino.h>
#include "math.h"

#include "commands.h"
#include "ledstrip.h"


bool ledState = false;
bool ledBlink = true;


void interpretCommand(const CommandInfo& ci)
{
	switch (ci.commandCode)
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
			FillCmd cmd(ci);
			strip0.Fill(cmd.color);
			break;
		}

		default:
			Serial.print("unhanded command ");
			Serial.println(char(ci.commandCode));
	}
}

// edit baud rate at runtime: https://forum.arduino.cc/index.php?topic=382040.0
void setup() {
	Serial.println(R"(==== setup ====)");

	pinMode(LED_BUILTIN, OUTPUT);
	// Serial.begin(3000000);
	Serial.begin(9600);
	ClearCurrentCommand();
	strip0.Setup();
	strip0.Clear();
	strip0.Apply();
}

void loop()
{
	Serial.println("****");

	ProcessInputSerialStream();
	CommandInfo ci;
	if (GetCommandInfo(ci))
	{
		interpretCommand(ci);
		ClearCurrentCommand();
		strip0.Apply(); // as the arduino api buffers the data
	}

	digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
	if (ledBlink) ledState = !ledState;
	delay(50);
}

