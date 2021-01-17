// johan.duparc

#include "RemoteDisplay.h"
#include "SenderSerial.h"

#include "fmt/color.h"

#include <thread>



using Arduino = std::unique_ptr<SerialPort>;


namespace detail 
{


std::unique_ptr<SenderInterface> scanSerialPort()
{
	for (int i = 0; i < 255; ++i)
	{
		fmt::print("scan com port {}:\n", i);

		fmt::memory_buffer port;
		format_to(port, "\\\\.\\COM{}", i);
		BaudRate Rate = BaudRate::BR_9600;
// 		BaudRate Rate = BaudRate(3000000);
		Arduino arduino = std::make_unique<SerialPort>(port.data(), Rate);

		if (bool isConnected = arduino->isConnected())
		{
			fmt::print("com port {} available!\n", i);
			return std::make_unique<SenderSerial>(std::move(arduino));
		}
	}

	return nullptr;
}

} // ns detail


bool RemoteDisplay::Connect()
{
	sender = detail::scanSerialPort();
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

void RemoteDisplay::Fill(const Color_24b& fillColor)
{
	sender->PushCommand(CommandFill(fillColor));
}

