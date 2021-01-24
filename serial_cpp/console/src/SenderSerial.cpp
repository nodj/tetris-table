// johan.duparc

#include "SenderSerial.h"

#include "fmt/format.h"
#include "SerialPort.hpp"
#include <utility>
#include <thread>
#include <vector>


// https://www.codeproject.com/Articles/2682/Serial-Communication-in-Windows


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
			break;
		}
	}

	freeByteAvailable = 0;
	if (Port)
	{
		syncThread = { [this] { SyncThreadTask(); } };
		freeByteAvailable = 63;
	}
}

SenderSerial::~SenderSerial()
{
	// notify to kill

	syncThreadRunnable = false;
	syncThread.join();
}

bool SenderSerial::CanSend()
{
	return isFine && Port && Port->isConnected();
}

void SenderSerial::PushBuffer(const uint8_t* buffer, uint32_t bufferLength)
{
	// todo: if the queue is empty, there is a useless copy.
	std::lock_guard lock(syncThreadMutex);
	std::copy(buffer, buffer + bufferLength, std::back_inserter(sendQueue));
}

bool SenderSerial::CanReceive()
{
	return true;
}

int SenderSerial::ReceiveBuffer(uint8_t* buffer, uint32_t bufferLength)
{
	memset(buffer, 0, bufferLength);
	std::lock_guard lock(syncThreadMutex);
	int32_t byteToMove = std::min(bufferLength, receiveQueue.size());

	std::copy(receiveQueue.data(), receiveQueue.data()+byteToMove, buffer);
	receiveQueue.erase(receiveQueue.begin(), receiveQueue.begin()+byteToMove);

	return byteToMove;
}

int32_t SenderSerial::Available()
{
	std::lock_guard lock(syncThreadMutex);
	return receiveQueue.size();
}

void SenderSerial::ProcessSendQueue()
{
	uint8_t byteToSend = (uint8_t)std::min(freeByteAvailable, sendQueue.size());

	if (byteToSend > 0)
	{
		isFine = Port->writeSerialPort(sendQueue.data(), byteToSend);
		freeByteAvailable -= byteToSend;
		sendQueue.erase(sendQueue.begin(), sendQueue.begin()+byteToSend);
	}
}

void SenderSerial::ProcessReceiveQueue()
{
	int32_t availableBytes = Port->available();
	auto initialSize = receiveQueue.size();
	receiveQueue.resize(receiveQueue.size() + availableBytes);
	int32_t actualRead = Port->readSerialPort(receiveQueue.data()+initialSize, availableBytes);
	if (actualRead != availableBytes)
	{
		receiveQueue.resize(initialSize + actualRead);
	}
// 	freeByteAvailable += actualRead; // wow, not at all...
	//rebudget by the read count as declared by the arduino
	if (actualRead > 0) freeByteAvailable = 63; // assume arduino has read everything... which is plain false
}

uint32_t SenderSerial::SendInternal(const void* buffer, uint32_t bufferLength)
{
	uint32_t byteCountToSend = std::min(freeByteAvailable, bufferLength);

	if (isFine = CanSend())
	{
		isFine = Port->writeSerialPort((uint8_t*)buffer, byteCountToSend);
	}

	return byteCountToSend;
}

void SenderSerial::SyncThreadTask()
{
	int i = 1;
	while (syncThreadRunnable)
	{
		std::unique_lock lock(syncThreadMutex);
		if (i == 1)
		{
			// polling strategy:
			ProcessReceiveQueue();

			// todo: could be event driven: https://www.codeproject.com/Articles/2682/Serial-Communication-in-Windows
		}
		else
		{
			ProcessSendQueue();
		}
		syncThreadCV.wait_for(lock, std::chrono::milliseconds(5));
		i = -i;
	}
}
