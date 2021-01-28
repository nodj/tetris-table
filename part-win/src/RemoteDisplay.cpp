// johan.duparc

#include "RemoteDisplay.h"
#include "SenderSerial.h"
#include "Command.h"

#include "fmt/color.h"


enum class CommandCode : uint8_t
{
	None          = 'a'-1,
	Fill          = 'a',
	LedBlink_void = 'b',
	LedOn_void    = 'c',
	LedOff_void   = 'd',
	
	_max
};

struct CommandFill : public Command
{
	Color_24b c;

	CommandFill(const Color_24b& color) : c(color) {}
	virtual CommandCode GetCode() const override final { return CommandCode::Fill; }
	virtual void Visit(IOBuffer& Buffer) override final
	{
		Buffer << c.R << c.G << c.B;
	}
};

struct CommandLedOff : public Command
{
	virtual CommandCode GetCode() const override final { return CommandCode::LedOff_void; }
};

struct CommandLedOn : public Command
{
	virtual CommandCode GetCode() const override final { return CommandCode::LedOn_void; }
};




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
	int32_t availableByteCount = sender->Available();
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

void PushCommand(std::unique_ptr<SenderInterface>& sender, const Command& command)
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
	PushCommand(sender, CommandFill(fillColor));
}

