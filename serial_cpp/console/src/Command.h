// johan.duparc

#pragma once 

#include <stdint.h>


enum class CommandCode : uint8_t;


struct Command
{
	virtual CommandCode GetCode() const = 0;
	virtual void Visit(class IOBuffer& Buffer) {}
};

