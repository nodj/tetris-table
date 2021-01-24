// johan.duparc

#pragma once
#include <stdint.h>


// abstract the transport layer (eg, Serial vs...)
class SenderInterface
{
public:
	// @returns wether the
	virtual bool CanSend() = 0;

	virtual void PushBuffer(const uint8_t* buffer, uint32_t bufferLength) = 0;
	virtual void PushBuffer(const void* buffer, uint32_t bufferLength)
	{
		PushBuffer((uint8_t*)buffer, bufferLength);
	}

	virtual bool CanReceive() = 0;

	// get the number of readable bytes
	virtual int32_t Available() = 0;

	// @returns: read byte count
	virtual int ReceiveBuffer(uint8_t* buffer, uint32_t bufferLength) = 0;

};

