// johan.duparc

#include "commands.h"


#define CHECK_BUFFER 1 // todo: optionnally skip checks

struct SerialReaderHelper
{
	// todo: copy all in an internal buffer, send ack to win, and then parse.
	// -> allows sender to send the following bytes sooner
	// -> prevents interleaved direct serial read which would break the available byte count

	SerialReaderHelper()
		: available(Serial.available())
	{
	}

	SerialReaderHelper& operator >> (uint16_t& u16)
	{
		uint8_t lo = Serial.read();
		uint8_t hi = Serial.read();
		available -= 2;
		u16 = hi << 8 | lo;
		parseOk &= available >= 0;
		return *this;
	}

	SerialReaderHelper& operator >> (uint8_t& u8)
	{
		u8 = Serial.read();
		available -= 1;
		parseOk &= available >= 0;
		return *this;
	}

	bool CanRead(int16_t byteCount) const { return available >= byteCount; }
	int16_t Available() const { return available; }
	int Peek() const { return Serial.peek(); }
	int16_t ReadAtMost(uint8_t* buffer, int16_t maxByteCount)
	{
		const int16_t readCount = max(min(available, maxByteCount), 0);
		if (readCount)
		{
			available -= readCount;
			for (int i = 0; i < readCount; ++i)
				buffer[i] = (uint8_t)Serial.read();
		}
		return readCount;
	}
	void SkipByte(int i) { while (i-- && available--) Serial.read(); }

private:
	int available;
	bool parseOk = true;
};


struct Header
{
	bool Parse(SerialReaderHelper& reader)
	{
		if (reader.CanRead(4) && reader.Peek() == '<')
		{
			reader.SkipByte(1);
			reader >> commandCode >> commandSize;

			return true;
		}
		return false;
	}

	uint16_t commandSize;
	uint8_t commandCode;
};

enum class CommandParsingState {
	None,
	HeaderOk,
	BufferOk,
	CommandOk,
};


//---------
CommandParsingState gParsingState;
Header gCurrentHeader;
CommandInfo gCurrentCommand;
uint32_t gCommandBufferWrote;
//---------


void FillCommandBuffer(SerialReaderHelper& reader)
{
	int16_t copiedByteCount = reader.ReadAtMost(gCurrentCommand.WritePtr(), gCurrentCommand.Slack());
	gCurrentCommand.writeOffset += copiedByteCount;
}


bool GetCommandInfo(CommandInfo& ci)
{
	if (gParsingState != CommandParsingState::CommandOk)
	{
		return false;
	}

	ci = gCurrentCommand;

	gCurrentCommand.buffer = nullptr; // dont dealloc just copied ptr... low cost std::move :/
	gCurrentCommand = CommandInfo();
	gCurrentHeader = Header();

	gParsingState = CommandParsingState::None;

	return true;
}

void DbgPrint(const char* txt) { Serial.println(txt); }

#define UNHANDLED_ERROR { Serial.print("unhandled Error: l: "); Serial.println(__LINE__); }

void ProcessInputSerialStream()
{
	SerialReaderHelper reader;
	bool continueReading = true;
	while (continueReading && reader.CanRead(1))
	{
		continueReading = false;
		switch(gParsingState)
		{
			case CommandParsingState::None:
			{
				// consume useless chars
				while (reader.CanRead(1) && reader.Peek() != '<')
				{
					reader.SkipByte(1);
				}

				// detect cmd start
				if (gCurrentHeader.Parse(reader))
				{
					Serial.println("> HeaderOk");
					Serial.print(">> h.cc:");
					Serial.println(gCurrentHeader.commandCode);
					Serial.print(">> h.bs:");
					Serial.println(gCurrentHeader.commandSize);
					gParsingState = CommandParsingState::HeaderOk;
					gCurrentCommand.code = CommandCode(gCurrentHeader.commandCode);
					if (gCurrentHeader.commandSize > 0)
					{
						gCurrentCommand.Alloc(gCurrentHeader.commandSize);
					}
					else
					{
						gParsingState = CommandParsingState::BufferOk;
					}
					continueReading = true;
				}
				break;
			}

			case CommandParsingState::HeaderOk:
			{
				FillCommandBuffer(reader);

				if (gCurrentCommand.Slack() == 0)
				{
					gParsingState = CommandParsingState::BufferOk;
					continueReading = true;
				}
				break;
			}

			case CommandParsingState::BufferOk:
			{
				if (reader.CanRead(1))
				{
					uint8_t closure;
					reader >> closure;
					if (closure == '>')
					{
						Serial.println("== Command completed !");
						gParsingState = CommandParsingState::CommandOk;
						continueReading = true;
					}
					else
					{
						UNHANDLED_ERROR;
					}
				}
				break;
			}

			case CommandParsingState::CommandOk:
			{
				// do nothing, wait for command to be consumed
				Serial.println("CommandOk");
				break;
			}
		} // switch
	} // while continue reading
} // fn
