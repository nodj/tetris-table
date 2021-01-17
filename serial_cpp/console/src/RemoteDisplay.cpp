// johan.duparc

#include "RemoteDisplay.h"
#include "SenderSerial.h"
#include "SerialPort.hpp"

#include "fmt/color.h"



bool RemoteDisplay::Connect()
{
	sender = std::make_unique<SenderSerial>();
	return IsConnected();
}


bool RemoteDisplay::IsConnected()
{
	return sender && sender->CanSend() && sender->CanReceive();
}

void RemoteDisplay::PrintSerialInput()
{
	std::string str;
	str.resize(512);
	int i = sender->ReceiveBuffer((void*)str.data(), str.length());
	str.resize(i);
	if (i)
	{
		fmt::print("received {} bytes:\n", str.length());
		fmt::print(fmt::fg(fmt::color::deep_sky_blue), str);
	}
}

void RemoteDisplay::PushCommand(const Command& command)
{
	IOBuffer ioBuffer;
	ioBuffer.buffer.reserve(64);
	const_cast<Command&>(command).Visit(ioBuffer);

	uint8_t header = '<';
	sender->PushBuffer(&header, 1);

	uint8_t code = (uint8_t)command.GetCode();
	sender->PushBuffer(&code, 1);

	uint16_t byteCount = static_cast<uint16_t>(ioBuffer.buffer.size());
	sender->PushBuffer(&byteCount, 2);

	sender->PushBuffer(ioBuffer.buffer.data(), byteCount);

	uint8_t footer = '>';
	sender->PushBuffer(&footer, 1);
}

void RemoteDisplay::Fill(const Color_24b& fillColor)
{
	PushCommand(CommandFill(fillColor));
}

