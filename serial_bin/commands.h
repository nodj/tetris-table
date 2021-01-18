// johan.duparc

#pragma once

#include <Arduino.h>

#include "common.h"

// #todo: move this in a file shared by both plateforms
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


struct CommandInfo
{
	CommandCode commandCode = CommandCode::None;
	uint8_t* commandBuffer = nullptr;
	uint32_t commandBufferSize = 0;
};

bool GetCommandInfo(CommandInfo& ci);
void ClearCurrentCommand();

// specific to this project
struct FillCmd
{
	Color_24b color;

	FillCmd(const CommandInfo& ci)
	{
		if (ci.commandBufferSize >= 3)
		{
			color.R = ci.commandBuffer[0];
			color.G = ci.commandBuffer[1];
			color.B = ci.commandBuffer[2];
		}
	}
};

