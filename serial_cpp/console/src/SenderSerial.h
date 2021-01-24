// johan.duparc

#pragma once

#include "SenderInterface.h"

#include <mutex>
#include <thread>
#include <vector>

// handle the serial communication with a remote.
// todo: handle packet splitting, acknowlegment, etc
// todo: handle answer reconbination
class SenderSerial
	: public SenderInterface
{
public:
	SenderSerial();
	~SenderSerial();

	virtual bool CanSend() override;
	virtual void PushBuffer(const uint8_t* buffer, uint32_t bufferLength) override;

	virtual bool CanReceive() override;
	virtual int32_t Available() override;
	virtual int ReceiveBuffer(uint8_t* buffer, uint32_t bufferLength) override;

private:
	void ProcessSendQueue();
	void ProcessReceiveQueue();
	uint32_t SendInternal(const void* buffer, uint32_t bufferLength);

	bool isFine = true;
	uint32_t freeByteAvailable;
	std::unique_ptr<class SerialPort> Port;

	// stategy 1:
	// sender (S) knows the receiver (R) buffer size (N, eg. 63 bytes).
	// S makes packets of at most N, send, deduce this from the sendBudget, and wait for the ack to recredit that to the buffer

	// stategy 2:
	// a 'CLEAR' command instruct to empty the R buffer, and expect an available byte count in answer.
	// S knowing the R buffer size can send just that, decrement the budget.
	// R read x bytes and notify that. S can increment the budget by that count.

	std::vector<std::uint8_t> sendQueue;
	std::vector<std::uint8_t> receiveQueue;

	// -> inner class
	void SyncThreadTask();
	std::atomic<bool> syncThreadRunnable{true};
	std::thread syncThread;
	std::mutex syncThreadMutex;
	std::condition_variable syncThreadCV;
};
