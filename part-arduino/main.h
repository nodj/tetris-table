// johan.duparc

#pragma once

#include <Arduino.h>
#include <math.h>

#include "shared/types.h"

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


void Setup()
{
	pinMode(LED_BUILTIN, OUTPUT);

	// Serial.begin(3000000);
	Serial.begin(9600);
	while (Serial.read() != -1); // flush Rx, just in case
	Serial.flush();
	delay(100);

	Serial.println(R"(==== setup ====)");
	DeclareSetup(SERIAL_RX_BUFFER_SIZE-1); // allows to reset the c++ part, flush serial buffers, etc...

	strip0.Setup();
	strip0.Clear();
	u8 b = 100;
	Color_24b c = {0, 0, b};
	strip0.Fill(c);
	strip0.Apply();
}


void Loop()
{
	Serial.println("~~");

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