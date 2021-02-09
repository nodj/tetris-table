// johan.duparc

#pragma once
#include <stdint.h>
#include <atomic>


// abstract the transport layer (eg, Serial vs...)
class SenderInterface
{
public:
	virtual ~SenderInterface() = default;

	// @returns: wether the PushBuffer methods are usable
	virtual bool CanSend() = 0;

	virtual void PushBuffer(const uint8_t* buffer, size_t bufferLength) = 0;
	void PushBuffer(const void* buffer, size_t bufferLength) { PushBuffer((uint8_t*)buffer, bufferLength); }

	virtual bool CanReceive() = 0;

	// @returns: number of readable bytes
	virtual size_t Available() = 0;

	// @param buffer: destination buffer
	// @param bufferLength: destination buffer capacity
	// @returns: read byte count
	virtual size_t ReceiveBuffer(uint8_t* buffer, size_t bufferLength) = 0;

	// used by the Serial sender to signal end-of-frame acknowledgment by the arduino
	std::atomic<bool> LastSegmentAck{true};
};
