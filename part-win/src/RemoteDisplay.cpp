// johan.duparc

#include "RemoteDisplay.h"
#include "SenderSerial.h"
#include "Command.h"

#include "fmt/color.h"

#include "shared/commands.h"
#include "shared/config.h"
#include <cassert>



template<CommandCode c>
struct TagCommand : public Command
{
	virtual CommandCode GetCode() const override final { return c; }
};

using CommandFrameBegin = TagCommand<CommandCode::FrameBegin>;
using CommandFrameEnd   = TagCommand<CommandCode::FrameEnd>;
using CommandLedOn      = TagCommand<CommandCode::LedOn_void>;
using CommandLedOff     = TagCommand<CommandCode::LedOff_void>;


struct CommandFill : public Command
{
	Color_24b color;

	CommandFill(const Color_24b& color) : color(color) {}
	virtual CommandCode GetCode() const override final { return CommandCode::Fill; }
	virtual void Visit(IOBuffer& buffer) override final
	{
		buffer << color;
	}
};


struct CommandSetPixel : public Command
{
	uint16_t index;
	Color_24b color;

	CommandSetPixel(uint16_t index, const Color_24b& color) : index(index), color(color) {}
	virtual CommandCode GetCode() const override final { return CommandCode::SetPixel; }

	virtual void Visit(IOBuffer& buffer) override final
	{
		buffer << index << color;
	}
};


bool RemoteDisplay::Connect()
{
	sender = std::make_unique<SenderSerial>(BAUD_RATE);
	return IsConnected();
}


bool RemoteDisplay::IsConnected()
{
	return sender && sender->CanSend() && sender->CanReceive();
}


void RemoteDisplay::PrintSerialInput()
{
	if (int32_t availableByteCount = sender->Available())
	{
		fmt::print("({} bytes available)\n", availableByteCount);
		std::string str;
		str.resize(availableByteCount);
		int i = sender->ReceiveBuffer((uint8_t*)str.data(), str.length());
		str.resize(i);
		if (i)
		{
			fmt::print("received {} bytes:\n", str.length());
			fmt::print(fmt::fg(fmt::color::deep_sky_blue), "{}", str);
		}
	}
}


void RemoteDisplay::PushCommand(const Command& command)
{
	GetWriteSegment().AddCommand(command);
	TrySendSegments();
}

void RemoteDisplay::TrySendSegments()
{
	if (segments.empty())
		return; // nothing to do

	if (segments[0].IsEmpty())
	{
		if (segments[0].Writable())
		{
			// empty but writable, nothing to do
			return;
		}
		else
		{
			// waiting for ack... Nothing to do either
			return;
		}
	}

	segments[0].Push(sender);
}

RemoteDisplay::Segment& RemoteDisplay::GetWriteSegment()
{
	return (segments.empty() || segments.back().Writable() == false)
		? segments.emplace_back()
		: segments.back();
}

void RemoteDisplay::CloseSegment()
{
	if (segments.empty()) return; // bad api warn: no open segment
	if (segments.back().Writable() == false) return; // bad api warn: already closed

	segments.back().Close();
}

// void PushCommandInternal(std::unique_ptr<SenderInterface>& sender, const Command& command)
// {
// 	IOBuffer ioBuffer;
// 	ioBuffer.buffer.reserve(64);
// 	const_cast<Command&>(command).Visit(ioBuffer);
//
// 	uint8_t header = '<';
// 	sender->PushBuffer(&header, 1);
//
// 	uint8_t code = (uint8_t)command.GetCode();
// 	sender->PushBuffer(&code, 1);
//
// 	uint16_t byteCount = static_cast<uint16_t>(ioBuffer.buffer.size());
// 	sender->PushBuffer(&byteCount, 2);
//
// 	sender->PushBuffer(ioBuffer.buffer.data(), byteCount);
//
// 	uint8_t footer = '>';
// 	sender->PushBuffer(&footer, 1);
// }


void RemoteDisplay::Fill(const Color_24b& fillColor)
{
	PushCommand(CommandFill(fillColor));
}


void RemoteDisplay::FrameBegin()
{
	PushCommand(CommandFrameBegin());
}


void RemoteDisplay::FrameEnd()
{
	PushCommand(CommandFrameEnd());
	CloseSegment();
}


void RemoteDisplay::SetPixel(int32_t stripIndex, const Color_24b& color)
{
	PushCommand(CommandSetPixel(stripIndex, color));
}

static uint8_t id_gen = 0;
RemoteDisplay::Segment::Segment()
	:id(id_gen++)
{
	buffer.reserve(512); // no idea if this size is smart.
}

void RemoteDisplay::Segment::AddCommand(const Command& command)
{
	buffer.reserve(buffer.size() + 32);
	IOBuffer ioBuffer(buffer);

	ioBuffer << '<';
	ioBuffer << uint8_t(command.GetCode());

	size_t payloadSizeOffset = buffer.size();
	ioBuffer << uint16_t(0);

	size_t byteCountBefore = buffer.size();
	const_cast<Command&>(command).Visit(ioBuffer);
	size_t byteCountAfter = buffer.size();
	ioBuffer << '>';

	// extended footer : detects how many bytes were dropped
	ioBuffer << '1' << '2';

	// fixup size
	size_t byteCount = byteCountAfter - byteCountBefore;
	assert(byteCount <= UINT16_MAX);
	buffer[payloadSizeOffset] = uint8_t(byteCount & 0xff);
	buffer[payloadSizeOffset+1] = uint8_t((byteCount>>8) & 0xff);
}

void RemoteDisplay::Segment::Push(std::unique_ptr<class SenderInterface>& sender)
{
	sender->PushBuffer(buffer.data(), buffer.size());
	buffer.clear();
// 	transmitting = true;
}
