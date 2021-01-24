// johan.duparc

#pragma once

#include <Arduino.h>

#include "common.h"

// #todo: move this in a file shared by all plateforms
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

