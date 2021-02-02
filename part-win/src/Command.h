// johan.duparc

#pragma once

#include "shared/types.h"

#include <vector>


class IOBuffer
{
public:
	template<typename T>
	IOBuffer& operator << (const T& arg);

// private:
	std::vector<uint8_t> buffer;
};

template<>
inline IOBuffer& IOBuffer::operator << (const uint8_t& arg)
{
	buffer.push_back(arg);
	return *this;
}

template<>
inline IOBuffer& IOBuffer::operator << (const uint16_t& arg)
{
	*this << uint8_t(arg & 0xff) << uint8_t((arg>>8) & 0xff);
	return *this;
}

template<>
inline IOBuffer& IOBuffer::operator << (const uint32_t& arg)
{
	*this << uint8_t(arg & 0xff) << uint8_t((arg>>8) & 0xff) << uint8_t((arg>>16) & 0xff) << uint8_t((arg>>24) & 0xff);
	return *this;
}

template<>
inline IOBuffer& IOBuffer::operator << (const Color_24b& arg)
{
	*this << arg.R << arg.G << arg.B;
	return *this;
}


enum CommandCode : u8;

class Command
{
public:
	virtual CommandCode GetCode() const = 0;
	virtual void Visit(class IOBuffer& buffer) {}
};
