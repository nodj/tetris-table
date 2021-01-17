// johan.duparc

#pragma once

#include "SenderInterface.h"


using ColorCode = uint8_t;

struct Color_24b { 
	uint8_t R = 127;
	uint8_t G = 0;
	uint8_t B = 0;
};




enum class CommandCode : uint8_t
{
	None          = 'a'-1,
	Fill          = 'a',
	LedBlink_void = 'b',
	LedOn_void    = 'c',
	LedOff_void   = 'd',
	
	_max
};

struct CommandFill : public Command
{
	Color_24b c;

	CommandFill(const Color_24b& color) : c(color) {}
	virtual CommandCode GetCode() const override final { return CommandCode::Fill; }
	virtual void Visit(IOBuffer& Buffer) override final
	{
		Buffer << c.R << c.G << c.B;
	}
};

struct CommandLedOff : public Command
{
	virtual CommandCode GetCode() const override final { return CommandCode::LedOff_void; }
};

struct CommandLedOn : public Command
{
	virtual CommandCode GetCode() const override final { return CommandCode::LedOn_void; }
};
