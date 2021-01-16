#include "Arduino.h"
#include "math.h"

#include "lib.h"

bool ledState = false;
bool ledBlink = true;


void interpretCommand(CommandCode command)
{
	switch (command)
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

		default:
			Serial.print("unhanded command ");
			Serial.println(char(command));
	}
}

// edit baud rate at runtime: https://forum.arduino.cc/index.php?topic=382040.0
void setup() {
	Serial.println("==== setup ====");

	pinMode(LED_BUILTIN, OUTPUT);
	// Serial.begin(3000000);
	Serial.begin(9600);
	ClearCurrentCommand();
}

void loop()
{
	Serial.println("==== loop ====");

	ProcessInputSerialStream();
	CommandInfo ci;
	if (GetCommandInfo(ci))
	{
		interpretCommand(ci.commandCode);
		ClearCurrentCommand();
	}

	digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
	if (ledBlink) ledState = !ledState;
	delay(500);
}

// void xxxx()
// {
// 	Serial.println("==== xxxx ====");
// }
