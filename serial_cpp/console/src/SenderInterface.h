// johan.duparc

#pragma once

#include "Command.h"

#include <memory>
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

	
// Api to send commands, abstract the transport layer (eg, Serial vs...)
class SenderInterface
{
public:
	virtual bool CanSend() = 0;
	virtual void PushBuffer(const void* buffer, uint32_t bufferLength) = 0;

	virtual bool CanReceive() = 0;

	// @returns: read byte count
	virtual int ReceiveBuffer(void* buffer, uint32_t bufferLength) = 0;

public:
	void PushCommand(const Command& command)
	{
		IOBuffer ioBuffer;
		ioBuffer.buffer.reserve(64);
		const_cast<Command&>(command).Visit(ioBuffer);
		
		uint8_t header = '<';
		PushBuffer(&header, 1);

		uint8_t code = (uint8_t)command.GetCode();
		PushBuffer(&code, 1);

		uint16_t byteCount = static_cast<uint16_t>(ioBuffer.buffer.size());
		PushBuffer(&byteCount, 2);

		PushBuffer(ioBuffer.buffer.data(), byteCount);

		uint8_t footer = '>';
		PushBuffer(&footer, 1);
	}

};

