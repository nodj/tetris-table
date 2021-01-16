// johan.duparc

#pragma once

#include <memory>
#include <vector>


using ColorCode = uint8_t;

enum CommandCode : uint8_t
{
	None          = 'a'-1,
	Fill          = 'a',
	LedBlink_void = 'b',
	LedOn_void    = 'c',
	LedOff_void   = 'd',
	
	CommandCode_max
};



struct IOBuffer
{
	template<typename T>
	IOBuffer& operator << (const T& arg);
	

	std::vector<uint8_t> buffer;
};


template<>
IOBuffer& IOBuffer::operator << (const uint8_t& arg)
{
	buffer.push_back(arg);
	return *this;
}

struct Command
{
	virtual CommandCode GetCode() const = 0;
	virtual void Visit(IOBuffer& Buffer) {}
};


struct Ack
{
	CommandCode code;
	bool isOk;
};
	

class SenderInterface
{
public:
	virtual bool CanSend() = 0;
	virtual void SendBuffer(const void* buffer, uint32_t bufferLength) = 0;

	virtual bool CanReceive() = 0;

	// returns: read byte count
	virtual int ReceiveBuffer(void* buffer, uint32_t bufferLength) = 0;

public:
	void PushCommand(const Command& command)
	{
		Consume();

		IOBuffer ioBuffer;
		const_cast<Command&>(command).Visit(ioBuffer);
		
		uint8_t header = '<';
		SendBuffer(&header, 1);

		uint8_t code = (uint8_t)command.GetCode();
		SendBuffer(&code, 1);

		uint16_t byteCount = static_cast<uint16_t>(ioBuffer.buffer.size());
		SendBuffer(&byteCount, 2);

		SendBuffer(ioBuffer.buffer.data(), byteCount);

		uint8_t footer = '>';
		SendBuffer(&footer, 1);
	}

	bool Receive(Ack& ack)
	{
		Consume();

	}
private:

	void Consume()
	{

	}
};

using ISenderRef = std::shared_ptr<SenderInterface>;


struct CommandFill : public Command
{
	uint8_t R, G, B;

	virtual CommandCode GetCode() const override final { return CommandCode::Fill; }
	virtual void Visit(IOBuffer& Buffer) override final
	{
		Buffer << R << G << B;
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
