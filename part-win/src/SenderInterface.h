// johan.duparc

#pragma once
#include <stdint.h>


// abstract the transport layer (eg, Serial vs...)
class SenderInterface
{
public:
	virtual ~SenderInterface() = default;

	// @returns: wether the PushBuffer methods are usable
	virtual bool CanSend() = 0;

	virtual void PushBuffer(const uint8_t* buffer, uint32_t bufferLength) = 0;
	void PushBuffer(const void* buffer, uint32_t bufferLength) { PushBuffer((uint8_t*)buffer, bufferLength); }

	virtual bool CanReceive() = 0;

	// @returns: number of readable bytes
	virtual int32_t Available() = 0;

	// @param buffer: destination buffer
	// @param bufferLength: destination buffer capacity
	// @returns: read byte count
	virtual int ReceiveBuffer(uint8_t* buffer, uint32_t bufferLength) = 0;
};
