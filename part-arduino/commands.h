// johan.duparc

#pragma once

#include <Arduino.h>

#include "shared/commands.h"
#include "SerialReaderHelper.h"

void ProcessInputSerialStream(SerialReaderHelper& reader);

struct CommandInfo
{
	CommandInfo(uint16_t size = 0) { Alloc(size); }
	~CommandInfo() { Dealloc(); }

	CommandCode code = CommandCode::None;
	uint8_t* buffer = nullptr;
	uint16_t bufferSize = 0;
	uint16_t writeOffset = 0;

	void Alloc(uint16_t size)
	{
		if (size > 0)
		{
			Dealloc();
			buffer = new uint8_t[size];
			bufferSize = size;
			writeOffset = 0;
		}
	}
	void Dealloc()
	{
		delete[] buffer;
		buffer = nullptr;
		bufferSize = 0;
	}
	int16_t Slack() const { return bufferSize - writeOffset; }
	uint8_t* WritePtr() const { return buffer + writeOffset; }
};

bool GetCommandInfo(CommandInfo& ci);
void ClearCurrentCommand();

// specific to this project
struct FillCmd
{
	Color_24b color;

	FillCmd(const CommandInfo& ci)
	{
		if (ci.bufferSize >= 3)
		{
			color.R = ci.buffer[0];
			color.G = ci.buffer[1];
			color.B = ci.buffer[2];
		}
	}
};

struct SetPixelCmd
{
	uint16_t index;
	Color_24b color;

	SetPixelCmd(const CommandInfo& ci)
	{
		if (ci.bufferSize >= 2+3)
		{
			index = *reinterpret_cast<uint16_t*>(ci.buffer);
			color.R = ci.buffer[2+0];
			color.G = ci.buffer[2+1];
			color.B = ci.buffer[2+2];
		}
	}
};

inline void DeclareConsumedSerialByte(uint8_t byteCount)
{
	Serial.write('<');
	Serial.write('R');
	Serial.write(byteCount);
	Serial.write('>');
}

inline void DeclareSetup(uint8_t rxBufferSize)
{
	Serial.write('<');
	Serial.write('S');
	Serial.write(rxBufferSize);
	Serial.write('>');
}

inline void DeclareConsumedFrame(int slack, int remaining)
{
	Serial.write('<');
	Serial.write('F');
	Serial.write(uint8_t(slack));
	Serial.write(uint8_t(remaining));
	Serial.write('>');
}
