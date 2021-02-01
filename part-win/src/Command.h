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


enum CommandCode : u8;

class Command
{
public:
	virtual CommandCode GetCode() const = 0;
	virtual void Visit(class IOBuffer& Buffer) {}
};
