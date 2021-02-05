// johan.duparc

#pragma once

#include "types.h"

enum CommandCode : u8
{
	None,
	Fill,
	LedBlink,
	LedOn,
	LedOff,

	FrameBegin, // Starts a buffered scope
	FrameEnd, // Ends the buffered scope, display content

	SetPixel,

	CommandCode_max
};



class IOBuffer
{
public:
	template<typename T> IOBuffer& operator << (      T& arg);
	template<typename T> IOBuffer& operator << (const T& arg) { return *this << const_cast<T&>(arg); }

protected:
	virtual IOBuffer& ProcessOneByte(uint8_t& arg) = 0;
};


template<typename T>
inline IOBuffer& IOBuffer::operator<<(T& arg)
{
	uint8_t* x = reinterpret_cast<uint8_t*>(&arg);
	for (int b = 0; b < int(sizeof(T)); ++b) ProcessOneByte(x[b]);
	return *this;
}



class Command
{
public:
	virtual CommandCode GetCode() const = 0;
	virtual void Visit(class IOBuffer&) {}
};



template<CommandCode c>
struct TagCommand : public Command
{
	virtual CommandCode GetCode() const override final { return c; }
};

using CommandFrameBegin = TagCommand<CommandCode::FrameBegin>;
using CommandFrameEnd   = TagCommand<CommandCode::FrameEnd>;
using CommandLedOn      = TagCommand<CommandCode::LedOn>;
using CommandLedOff     = TagCommand<CommandCode::LedOff>;

struct CommandSetPixel : public Command
{
	uint16_t index = 0;
	Color_24b color;

	CommandSetPixel() = default;
	CommandSetPixel(uint16_t index, const Color_24b& color) : index(index), color(color) {}
	virtual CommandCode GetCode() const override final { return CommandCode::SetPixel; }
	virtual void Visit(IOBuffer& buffer) override final
	{
		buffer << index;
		buffer << color;
	}
};

struct CommandFill : public Command
{
	Color_24b color;

	CommandFill() = default;
	CommandFill(const Color_24b& color) : color(color) {}
	virtual CommandCode GetCode() const override final { return CommandCode::Fill; }
	virtual void Visit(IOBuffer& buffer) override final
	{
		buffer << color;
	}
};
