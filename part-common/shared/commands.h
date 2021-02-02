// johan.duparc

#pragma once

#include "types.h"

enum CommandCode : u8
{
	None,
	Fill,
	LedBlink_void,
	LedOn_void,
	LedOff_void,

	FrameBegin, // Starts a buffered scope
	FrameEnd, // Ends the buffered scope, display content

	SetPixel,

	CommandCode_max
};

