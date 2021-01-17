// johan.duparc

#pragma once
#include <stdint.h>


// abstract the transport layer (eg, Serial vs...)
class SenderInterface
{
public:
	// @returns wether the 
	virtual bool CanSend() = 0;

	virtual void PushBuffer(const void* buffer, uint32_t bufferLength) = 0;

	virtual bool CanReceive() = 0;

	// @returns: read byte count
	virtual int ReceiveBuffer(void* buffer, uint32_t bufferLength) = 0;
};

