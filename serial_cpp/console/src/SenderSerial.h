// johan.duparc

#pragma once

#include "SenderInterface.h"


class SenderSerial 
	: public SenderInterface
{
public:
	SenderSerial(std::unique_ptr<SerialPort>&& Port) 
		: Port(std::move(Port))
	{
	}

	virtual bool CanSend() override
	{
		return isFine && Port && Port->isConnected();
	}

	virtual void SendBuffer(const void* buffer, uint32_t bufferLength) override
	{
		if (isFine = CanSend())
		{
			isFine = Port->writeSerialPort((uint8_t*)buffer, bufferLength);
		}
	}

	virtual bool CanReceive() override
	{
		return true;
	}

	virtual int ReceiveBuffer(void* buffer, uint32_t bufferLength) override
	{
		memset(buffer, 0, bufferLength);
		return Port->readSerialPort((uint8_t*)buffer, bufferLength);
	}

private:
	bool isFine = true;
	std::unique_ptr<SerialPort> Port;
};