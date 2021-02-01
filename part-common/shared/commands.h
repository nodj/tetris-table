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

	CommandCode_max
};

