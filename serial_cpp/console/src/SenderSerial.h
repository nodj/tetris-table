// johan.duparc

#pragma once

#include "SenderInterface.h"

#include <memory>


class SenderSerial 
	: public SenderInterface
{
public:
	SenderSerial();

	virtual bool CanSend() override;

	virtual void PushBuffer(const void* buffer, uint32_t bufferLength) override;

	virtual bool CanReceive() override;

	virtual int ReceiveBuffer(void* buffer, uint32_t bufferLength) override;

private:
	bool isFine = true;
	std::unique_ptr<class SerialPort> Port;
};