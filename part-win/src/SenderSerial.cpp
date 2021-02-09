// johan.duparc

#include "SenderSerial.h"

#include "fmt/format.h"
#include "fmt/color.h"
#include "SerialPort.hpp"

#include <cassert>
#include <thread>
#include <utility>
#include <vector>


// https://www.codeproject.com/Articles/2682/Serial-Communication-in-Windows

struct
{
	bool printTrashedBytes = false;
	int silentMsThreshold = 2;
	bool printCmdChars = true;
	bool printAlphaChars = true;
	bool queueAlphaChars = false;
} gConfig;


int32_t GetCommandPayloadByteCount(uint8_t b)
{
	if (b == 'R' || b == 'S') return 1;
	if (b == 'F') return 2;
	return 0;
}


SenderSerial::SenderSerial(int32_t baudRate)
	: freeByteAvailable(0)
{
	for (int i = 0; i < 255; ++i)
	{
		fmt::print("scan com port {}:\n", i);

		fmt::memory_buffer port;
		format_to(port, "\\\\.\\COM{}", i);
		std::unique_ptr<SerialPort> arduino = std::make_unique<SerialPort>(port.data(), baudRate);

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
			size_t actualRead = Port->readSerialPort(readBuffer.data(), (uint32_t)readBuffer.size());
			if (gConfig.printTrashedBytes)
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

void SenderSerial::PushBuffer(const uint8_t* buffer, size_t bufferLength)
{
	// todo: if the queue is empty, there is a useless copy.
	std::lock_guard lock(syncThreadMutex);
	sendQueue.reserve(sendQueue.size() + bufferLength);
	std::copy(buffer, buffer + bufferLength, std::back_inserter(sendQueue));
}

bool SenderSerial::CanReceive()
{
	return true;
}

size_t SenderSerial::ReceiveBuffer(uint8_t* buffer, size_t bufferLength)
{
	memset(buffer, 0, bufferLength);
	std::lock_guard lock(syncThreadMutex);
	size_t byteToMove = std::min(bufferLength, receiveQueue.size());
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

size_t SenderSerial::Available()
{
	std::lock_guard lock(syncThreadMutex);
	return receiveQueue.size();
}

void SenderSerial::ProcessSendQueue()
{
	uint8_t byteToSend = (uint8_t)std::min(std::min(freeByteAvailable, (size_t)255), sendQueue.size());

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
		fmt::print(fmt::fg(fmt::color::green), "received 'R' cmd. fba {} -> {}\n", freeByteAvailable, freeByteAvailable+cmd[1]);
		freeByteAvailable += cmd[1];
	}
	else if (cmd.size() == 2 && cmd[0] == 'S') // setup. buffer size
	{
		fmt::print(fmt::fg(fmt::color::green), "Setup command received:\n\tRx buffer: {} bytes", cmd[1]);
		freeByteAvailable = cmd[1];
	}
	else if (cmd.size() == 3 && cmd[0] == 'F') // setup. buffer size
	{
		fmt::print(fmt::fg(fmt::color::green), "Frame Ack received:\n\tRx buffer: {} bytes", cmd[1]);

		if (cmd[2])
			fmt::print(fmt::fg(fmt::color::red), " (unread:{})", cmd[2]);

		//freeByteAvailable = cmd[1]; // No ! as currently we know we will get a <R> msg just after
		if (LastSegmentAck.exchange(true))
		{
			fmt::print(fmt::fg(fmt::color::red), "ACK received, previous was ignored.");
		}
	}
}

void SenderSerial::ProcessReceiveQueue()
{
	// copy all in a tmp buff
	int32_t availableBytes = Port->available();
	if (availableBytes == 0)
		return;
// 	fmt::print("\nReceive {} bytes\n", availableBytes);
	std::vector<uint8_t> readBuffer;
	readBuffer.resize(availableBytes);
	int32_t actualRead = Port->readSerialPort(readBuffer.data(), (uint32_t)readBuffer.size());


	////////////////////////////////////////////////////////////////
	enum class ParseState
	{
		ASCII, // nothing special
		CMD, // following bytes -> cmd buffer
	};
	static ParseState ps = ParseState::ASCII;
	static int32_t cmdByteCount = 0;
	static int32_t commandPayloadByteCount = 0;
	static std::vector<uint8_t> cmdBuffer;
	////////////////////////////////////////////////////////////////

	auto initialSize = receiveQueue.size();
	receiveQueue.reserve(receiveQueue.size() + availableBytes);

	auto PrintCmdChar = [&](char c) { if (gConfig.printCmdChars) fmt::print(fmt::fg(fmt::color::green),"{}", c); };
	auto HandleStdChar = [&](uint8_t b) {
		if (gConfig.printAlphaChars)
			fmt::print(fmt::fg(fmt::color::blue),"{}", char(b));
		if (gConfig.queueAlphaChars)
			receiveQueue.push_back(b);
	};

	for (uint8_t b : readBuffer)
	{
		if (ps == ParseState::ASCII)
		{
			if (b == '<')
			{
				ps = ParseState::CMD;
				cmdByteCount = 0;

				cmdBuffer.clear();
				cmdBuffer.reserve(16);
				PrintCmdChar(b);
				continue;
			}
			HandleStdChar(b);
		}
		else if (ps == ParseState::CMD)
		{

			if (cmdByteCount == 0)
			{
				commandPayloadByteCount = GetCommandPayloadByteCount(b);
				cmdBuffer.reserve(commandPayloadByteCount + 1);
			}

			cmdByteCount++;

			if (cmdByteCount >= commandPayloadByteCount + 2) // +1 for the cmd code, +1 because we're one past the last payload byte
			{
				if (b == '>')
				{
					PrintCmdChar(b);
					ProcessReceivedCommand(cmdBuffer);
				}
				else
				{
					fmt::print(fmt::fg(fmt::color::red), "command parsing error.");
				}
				cmdBuffer.clear();
				ps = ParseState::ASCII;
				continue;
			}
			else
			{
				cmdBuffer.push_back(b);
				PrintCmdChar(b);
				continue;
			}
			HandleStdChar(b);
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

