// johan.duparc

#pragma once

#include <Arduino.h>
#include <math.h>

#include "shared/config.h"
#include "shared/types.h"

#include "commands.h"
#include "ledstrip.h"
#include "SerialReaderHelper.h"


bool ledState = false;
bool ledBlink = true;
bool bufferedMode = false;


void interpretCommand(const CommandInfo& command)
{
	Serial.print("interpretCommand:");
	Serial.println(command.code);

	switch (command.code)
	{
// 		case CommandCode::LedOn_void:
// 			ledState = true;
// 			ledBlink = false;
// 			break;
//
// 		case CommandCode::LedOff_void:
// 			ledState = false;
// 			ledBlink = false;
// 			break;
//
// 		case CommandCode::LedBlink_void:
// 			ledBlink = true;
// 			break;
//
// 		case CommandCode::Fill:
// 		{
// 			FillCmd fill(command);
// 			strip0.Fill(fill.color);
// 			break;
// 		}

		case CommandCode::FrameBegin:
		{
			bufferedMode = true;
			break;
		}

		case CommandCode::FrameEnd:
		{
			bufferedMode = false;
			// We expect a serial bus fully consumed now. We should be the last command before a stall
			strip0.Apply();
			int remaining = Serial.available();
			int slack = SERIAL_RX_BUFFER_SIZE - 1 - remaining;
			DeclareConsumedFrame(slack, remaining); // unblock the serial bus
			break;
		}

		case CommandCode::SetPixel:
		{
			SetPixelCmd cmd(command);
			strip0.SetPixel(cmd.index, cmd.color);
			break;
		}

		default:
			Serial.print("unhanded command ");
			Serial.println(char(command.code));
	}


	bool isBufferableCommand = false;
	switch (command.code)
	{
		case CommandCode::LedOn_void:
		case CommandCode::LedOff_void:
		case CommandCode::Fill:
		case CommandCode::SetPixel:
			isBufferableCommand = true;
		default:;
	}

	if (isBufferableCommand && !bufferedMode)
	{
// 		strip0.Apply();
	}
}


void Setup()
{
	pinMode(LED_BUILTIN, OUTPUT);

	Serial.begin(BAUD_RATE);
	while (Serial.read() != -1); // flush Rx, just in case
	delay(20); //

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

	SerialReaderHelper reader;
	ProcessInputSerialStream(reader);
	CommandInfo command;
	while (GetCommandInfo(command))
	{
		interpretCommand(command);
		ProcessInputSerialStream(reader);
	}

	digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
	if (ledBlink) ledState = !ledState;
	delay(50);
}
