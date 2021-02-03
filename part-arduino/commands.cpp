// johan.duparc

#include "commands.h"


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

void ProcessInputSerialStream(SerialReaderHelper& reader)
{
	bool continueReading = true;
	while (continueReading && reader.CanRead(1))
	{
		continueReading = false;
		switch (gParsingState)
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
					Serial.print("HeaderOk: cc:");
					Serial.print(gCurrentHeader.commandCode);
					Serial.print(" cs:");
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
						Serial.print("expected '>' char, got '");
						Serial.print(closure);
						Serial.println("'.");
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
