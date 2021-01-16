
#include "Arduino.h"

#pragma once


enum CommandCode : uint8_t
{
	None          = 'a'-1,
	Fill          = 'a',
	LedBlink_void = 'b',
	LedOn_void    = 'c',
	LedOff_void   = 'd',

	CommandCode_max
};


void ProcessInputSerialStream();

void ProcessCommand(uint8_t CommandCode);


struct CommandInfo
{
	CommandCode commandCode = CommandCode::None;
	uint8_t* commandBuffer = nullptr;
	uint32_t commandBufferSize = 0;
};
bool GetCommandInfo(CommandInfo& ci);
void ClearCurrentCommand();
