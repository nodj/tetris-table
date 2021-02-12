// This file is part of the tetris-table project. Copyright (c) Johan Duparc.

#include "commands.h"
#include "log.h"


#define UNHANDLED_ERROR { logE("unhandled Error: l: "); logEln(__LINE__); }


void CommandParser::ProcessInputSerialStream(SerialReaderHelper& reader)
{
	while (reader.CanRead(1) && ProcessInputSerialStreamImpl(reader));
}

bool CommandParser::ProcessInputSerialStreamImpl(SerialReaderHelper& reader)
{
	switch (parsingState)
	{
		case CommandParsingState::None:
		{
			// consume useless chars
			while (reader.CanRead(1) && reader.Peek() != '<')
			{
				reader.SkipByte(1);
			}

			// detect cmd start
			if (cmdBuffer.header.Parse(reader))
			{
				logV("HeaderOk: cc:"); logV(cmdBuffer.header.commandCode);
				logV(" cs:"); logVln(cmdBuffer.header.commandSize);
				cmdBuffer.writeOffset = 0;

				if (cmdBuffer.header.commandSize > CommandBuffer::MaximumPayloadSize)
				{
					logF("can't handle 256+ bytes commands. size:");
					logFln(cmdBuffer.header.commandSize);
					assert(0); // todo
				}
				else
				{
					parsingState = CommandParsingState::HeaderOk;
					return true;
				}
			}
			break;
		}

		case CommandParsingState::HeaderOk:
		{
			cmdBuffer.FillBuffer(reader);
			if (cmdBuffer.IsComplete())
			{
				parsingState = CommandParsingState::BufferOk;
				return true;
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
					logV("== Command completed !");
					parsingState = CommandParsingState::CommandOk;
					return true;
				}
				else
				{
					UNHANDLED_ERROR;
					logE("expected '>' char, got '");
					logE(closure);
					logEln("'.");
				}
			}
			break;
		}

		case CommandParsingState::CommandOk:
		{
			// do nothing, wait for command to be consumed
			logVln("CommandOk");
			break;
		}
	}
	return false;
}


void CommandBuffer::FillBuffer(SerialReaderHelper& reader)
{
	writeOffset += reader.ReadAtMost(&payload[writeOffset], header.commandSize - writeOffset);
}
