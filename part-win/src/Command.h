// johan.duparc

#pragma once

#include "shared/types.h"

#include <vector>


class IOBuffer
{
public:
	IOBuffer(std::vector<uint8_t>& buffer) : buffer(buffer) {}

	template<typename T>
	IOBuffer& operator << (const T& arg);

private:
	std::vector<uint8_t>& buffer;
};

template<>
inline IOBuffer& IOBuffer::operator << (const uint8_t& arg)
{
	buffer.push_back(arg);
	return *this;
}

template<>
inline IOBuffer& IOBuffer::operator << (const char& arg)
{
	return *this << uint8_t(arg);
}

template<>
inline IOBuffer& IOBuffer::operator << (const uint16_t& arg)
{
	return *this << uint8_t(arg & 0xff) << uint8_t((arg>>8) & 0xff);
}

template<>
inline IOBuffer& IOBuffer::operator << (const uint32_t& arg)
{
	return *this << uint8_t(arg & 0xff) << uint8_t((arg>>8) & 0xff) << uint8_t((arg>>16) & 0xff) << uint8_t((arg>>24) & 0xff);
}

template<>
inline IOBuffer& IOBuffer::operator << (const Color_24b& arg)
{
	return *this << arg.R << arg.G << arg.B;
}


enum CommandCode : u8;

class Command
{
public:
	virtual CommandCode GetCode() const = 0;
	virtual void Visit(class IOBuffer& buffer) {}
};
