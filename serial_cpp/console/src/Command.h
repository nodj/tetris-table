// johan.duparc

#pragma once 

#include <stdint.h>
#include <vector>


enum class CommandCode : uint8_t;


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

	

struct Command
{
	virtual CommandCode GetCode() const = 0;
	virtual void Visit(class IOBuffer& Buffer) {}
};
