// This file is part of the tetris-table project. Copyright (c) Johan Duparc.

#pragma once

#include <Arduino.h>
#include <math.h>

#include "shared/config.h"
#include "shared/types.h"

#include "commands.h"
#include "ledstrip.h"
#include "SerialReaderHelper.h"
#include "log.h"


bool ledState = false;
bool ledBlink = true;
bool bufferedMode = false;



struct IBuffer_FromStatic: public IOBuffer
{
	IBuffer_FromStatic(const uint8_t* buffer, int length)
		: buffer(buffer)
		, available(length)
	{}

private:
	virtual IOBuffer& ProcessOneByte(uint8_t& arg) override
	{
		arg = *buffer;
		buffer++;
		available--;
		return *this;
	}

public:
	const uint8_t* buffer;
	int available;
};


void InterpretCommand(const CommandBuffer& command)
{
	CommandCode cc = command.header.commandCode;

	logV("interpretCommand:");logVln(cc);

	switch (cc)
	{
		case CommandCode::LedOn:
			ledState = true;
			ledBlink = false;
			break;

		case CommandCode::LedOff:
			ledState = false;
			ledBlink = false;
			break;

		case CommandCode::LedBlink:
			ledBlink = true;
			break;

		case CommandCode::Fill:
		{
			IBuffer_FromStatic buffer(&command.payload[0], command.writeOffset);
			CommandFill fill;
			fill.Visit(buffer);
			strip0.Fill(fill.color);
			break;
		}

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
			IBuffer_FromStatic buffer(&command.payload[0], command.writeOffset);
			CommandSetPixel cmd;
			cmd.Visit(buffer);
			strip0.SetPixel(cmd.index, cmd.color);
			break;
		}

		default:
			logW("unhanded command "); logWln(char(cc));
	}

	bool isBufferableCommand = false;
	switch (cc)
	{
		case CommandCode::LedOn:
		case CommandCode::LedOff:
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

	logW(R"(==== setup ====)");
	DeclareSetup(SERIAL_RX_BUFFER_SIZE-1); // allows to reset the c++ part, flush serial buffers, etc...

	strip0.Setup();
	strip0.Clear();
	u8 b = 100;
	Color_24b c = {0, 0, b};
	strip0.Fill(c);
	strip0.Apply();
}


bool TryConsumeCommand(SerialReaderHelper& reader, CommandParser& parser)
{
	parser.ProcessInputSerialStream(reader);
	if (parser.GetCommand())
	{
		InterpretCommand(*parser.GetCommand());

		parser.NextCommand();
		return true;
	}
	return false;
}


void Loop()
{
	logVln("~~");

	SerialReaderHelper reader;
	static CommandParser parser;
	while (TryConsumeCommand(reader, parser));

	digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
	if (ledBlink) ledState = !ledState;
	delay(5);
}
