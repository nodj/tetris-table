// johan.duparc

#include "SenderSerial.h"

#include "fmt/format.h"
#include "SerialPort.hpp"


SenderSerial::SenderSerial()
{
	for (int i = 0; i < 255; ++i)
	{
		fmt::print("scan com port {}:\n", i);

		fmt::memory_buffer port;
		format_to(port, "\\\\.\\COM{}", i);
		BaudRate Rate = BaudRate::BR_9600;
// 		BaudRate Rate = BaudRate(3000000);
		std::unique_ptr<SerialPort> arduino = std::make_unique<SerialPort>(port.data(), Rate);

		if (bool isConnected = arduino->isConnected())
		{
			fmt::print("com port {} available!\n", i);
			Port = std::move(arduino);
			return;
		}
	}
}

bool SenderSerial::CanSend()
{
	return isFine && Port && Port->isConnected();
}

void SenderSerial::PushBuffer(const void* buffer, uint32_t bufferLength)
{
	if (isFine = CanSend())
	{
		isFine = Port->writeSerialPort((uint8_t*)buffer, bufferLength);
	}
}

bool SenderSerial::CanReceive()
{
	return true;
}

int SenderSerial::ReceiveBuffer(void* buffer, uint32_t bufferLength)
{
	memset(buffer, 0, bufferLength);
	return Port->readSerialPort((uint8_t*)buffer, bufferLength);
}
