// johan.duparc

#pragma once

#include <Arduino.h>

#include "shared/commands.h"
#include "SerialReaderHelper.h"
#include <assert.h>


struct Header
{
	bool Parse(SerialReaderHelper& reader)
	{
		if (reader.CanRead(4) && reader.Peek() == '<')
		{
			reader.SkipByte(1);
			reader >> (u8&)commandCode >> commandSize;
			return true;
		}
		return false;
	}

	uint16_t commandSize;
	CommandCode commandCode;
};

struct CommandBuffer
{
	enum {MaximumPayloadSize = 256};
	Header header;
	uint8_t payload[MaximumPayloadSize];
	uint16_t writeOffset = 0;

	void FillBuffer(SerialReaderHelper& reader);
	bool IsComplete() const { return writeOffset >= header.commandSize; }
};

struct CommandParser
{
	void ProcessInputSerialStream(SerialReaderHelper& reader);
	const CommandBuffer* GetCommand() const { return parsingState == CommandParsingState::CommandOk ? &cmdBuffer : nullptr; }
	void NextCommand() { parsingState = CommandParsingState::None; }

private:
	bool ProcessInputSerialStreamImpl(SerialReaderHelper& reader);
	enum class CommandParsingState {
		None,
		HeaderOk,
		BufferOk,
		CommandOk,
	};

	CommandParsingState parsingState = CommandParsingState::None;
	CommandBuffer cmdBuffer;
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
