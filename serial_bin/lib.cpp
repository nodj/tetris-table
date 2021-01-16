#include "lib.h"



///////////////////////////////////////////////////////////////
bool parseOk = true;

void ReadNext(uint16_t& u16, int& available)
{
	uint8_t lo = Serial.read();
	uint8_t hi = Serial.read();
	available -= 2;
	u16 = hi << 8 | lo;
	parseOk &= available >= 0;
}

void ReadNext(uint8_t& u8, int& available)
{
	u8 = Serial.read();
	available -= 1;
	parseOk &= available >= 0;
}
///////////////////////////////////////////////////////////////


struct Header
{
	bool Parse(int available)
	{
		if (available < 4)
			return false;
		Serial.read();
		ReadNext(commandCode, available);
		ReadNext(commandSize, available);
		return true;
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

CommandParsingState ParsingState;
Header CurrentHeader;

//---------
uint8_t* commandBuffer;
uint32_t commandBufferSize;
uint32_t commandBufferWrote;

//---------

void FillCmdBuffer(int available)
{
	int bufferSlack = commandBufferSize - commandBufferWrote;
	int byteToBuffer = min(available, bufferSlack);
	while (byteToBuffer--)
	{
		commandBuffer[commandBufferWrote++] = Serial.read();
	}
}


bool GetCommandInfo(CommandInfo& ci)
{
	if (ParsingState != CommandParsingState::CommandOk)
	{
		return false;
	}
	ci.commandCode = CommandCode(CurrentHeader.commandCode);
	ci.commandBuffer = commandBuffer;
	ci.commandBufferSize = commandBufferSize;
	return true;
}

void ClearCurrentCommand()
{
	ParsingState = CommandParsingState::None;

	commandBufferSize = 0;
	commandBufferWrote = 0;
	delete commandBuffer;
	commandBuffer = nullptr;
}

void ProcessInputSerialStream()
{
	int available = Serial.available();
	if (available)
	{
		switch(ParsingState)
		{
			case CommandParsingState::None:
			{
				int next = Serial.peek();

				// consume useless chars
				while (available && next != '<')
				{
					Serial.read();
					--available;
					next = Serial.peek();
				}

				// detect cmd start
				if (next == '<')
				{
					Serial.println("peeked <");
					if (CurrentHeader.Parse(available))
					{
						if (CurrentHeader.commandSize > 0)
						{
							ParsingState = CommandParsingState::HeaderOk;

							commandBufferSize = CurrentHeader.commandSize;
							commandBufferWrote = 0;
							delete commandBuffer;
							commandBuffer = new uint8_t[CurrentHeader.commandSize];
						}
						else
						{
							ParsingState = CommandParsingState::BufferOk;
						}
					}
				}
				break;
			}

			case CommandParsingState::HeaderOk:
			{
				Serial.println("== CommandParsingState::HeaderOk");
				Serial.print("h.cc:");
				Serial.println(CurrentHeader.commandCode);
				Serial.print("h.bs:");
				Serial.println(CurrentHeader.commandSize);
				FillCmdBuffer(available);
				if (commandBufferSize == commandBufferWrote)
				{
					ParsingState = CommandParsingState::BufferOk;
				}
				break;
			}

			case CommandParsingState::BufferOk:
			{
				Serial.println("== CommandParsingState::BufferOk");
				if (available)
				{
					uint8_t closure = Serial.read();
					if (closure == '>')
					{
						Serial.println("== Command completed !");
						ParsingState = CommandParsingState::CommandOk;
					}
				}
				break;
			}

			case CommandParsingState::CommandOk:
			{
				// do nothing, wait for command to be consumed
				Serial.println("== CommandParsingState::CommandOk");
				break;
			}
		}
	}

}
