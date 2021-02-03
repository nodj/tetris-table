// johan.duparc

#include "SenderSerial.h"

#include "fmt/format.h"
#include "SerialPort.hpp"

#include <cassert>
#include <thread>
#include <utility>
#include <vector>


// https://www.codeproject.com/Articles/2682/Serial-Communication-in-Windows

struct
{
	bool displayTrashedBytes = true;
	int silentMsThreshold = 2;
} gConfig;


SenderSerial::SenderSerial()
	: freeByteAvailable(0)
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

	if (Port)
	{
		// flush old remaining data
		while (int32_t availableBytes = Port->available())
		{
			fmt::print("SenderSerial: trash {} bytes\n", availableBytes);
			std::vector<uint8_t> readBuffer;
			readBuffer.resize(availableBytes);
			int32_t actualRead = Port->readSerialPort(readBuffer.data(), readBuffer.size());
			if (gConfig.displayTrashedBytes)
			{
				fmt::print("TRASH ({} bytes):\n", actualRead);
				for (uint8_t b : readBuffer)
					fmt::print("{}", char(b));
				fmt::print("-----\n");
			}

			// We don't immediately leave, as the serial port could have more bytes to trash.
			std::this_thread::sleep_for(std::chrono::milliseconds(gConfig.silentMsThreshold));
			// Note: It implies at least a few 'silent' milliseconds on the bus in order to leave this loop.
			// Concretely, the peer have to wait a few ms before sending us content.
			// Otherwise, that content would be considered Trash...
		}

		syncThread = { [this] { SyncThreadTask(); } };
	}
}

SenderSerial::~SenderSerial()
{
	if (syncThread.joinable())
	{
		// notify to kill
		syncThreadRunnable = false;
		syncThread.join();
	}
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
	if (byteToMove)
	{
		std::copy(receiveQueue.data(), receiveQueue.data()+byteToMove, buffer);
		if (receiveQueue.size() == byteToMove)
			receiveQueue.clear();
		else
			receiveQueue.erase(receiveQueue.begin(), receiveQueue.begin()+byteToMove);
	}
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
		if (sendQueue.size() == byteToSend)
			sendQueue.clear();
		else
			sendQueue.erase(sendQueue.begin(), sendQueue.begin()+byteToSend);
	}
}

void SenderSerial::ProcessReceivedCommand(const std::vector<uint8_t>& cmd)
{
	if (cmd.size() == 2 && cmd[0] == 'R') // consumer Rx byte count
	{
		freeByteAvailable += cmd[1];
	}
	else if (cmd.size() == 2 && cmd[0] == 'S') // setup. buffer size
	{
		fmt::print("Setup command received:\n");
		fmt::print("\tRx buffer: {} bytes", cmd[1]);
		freeByteAvailable = cmd[1];
	}
}

void SenderSerial::ProcessReceiveQueue()
{
	// copy all in a tmp buff
	int32_t availableBytes = Port->available();
	std::vector<uint8_t> readBuffer;
	readBuffer.resize(availableBytes);
	int32_t actualRead = Port->readSerialPort(readBuffer.data(), readBuffer.size());


	////////////////////////////////////////////////////////////////
	enum class ParseState
	{
		ASCII, // nothing special
		CMD, // following bytes -> cmd buffer
	};
	static ParseState ps = ParseState::ASCII;
	static std::vector<uint8_t> cmdBuffer;
	////////////////////////////////////////////////////////////////

	auto initialSize = receiveQueue.size();
	receiveQueue.reserve(receiveQueue.size() + availableBytes);
	for (uint8_t b : readBuffer)
	{
		if (ps == ParseState::ASCII)
		{
			if (b == '<')
			{
				ps = ParseState::CMD;
				cmdBuffer.clear();
				cmdBuffer.reserve(16);
				continue;
			}
			receiveQueue.push_back(b);
		}
		else if (ps == ParseState::CMD)
		{
			if (b == '>')
			{
				ps = ParseState::ASCII;
				ProcessReceivedCommand(cmdBuffer);
				cmdBuffer.clear();
				continue;
			}
			receiveQueue.push_back(b);//////// also print
			cmdBuffer.push_back(b);
		}
		else
		{
			assert(0);
		}
	}

}

void SenderSerial::SyncThreadTask()
{
	while (syncThreadRunnable)
	{
		std::unique_lock lock(syncThreadMutex);

		// polling strategy:
		ProcessReceiveQueue();
		ProcessSendQueue();

		// todo: could be event driven: https://www.codeproject.com/Articles/2682/Serial-Communication-in-Windows
		syncThreadCV.wait_for(lock, std::chrono::milliseconds(5));
	}
}
