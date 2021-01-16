// johan.duparc

#include "SerialPort.hpp"

// https://fmt.dev/latest/index.html
// rgb terminal require HKCU/Console/VirtualTerminalLevel:DWORD=1
#include "fmt/format.h"
#include "fmt/color.h"

#include "SenderSerial.h"

#include <string>
#include <iostream>


int read(std::unique_ptr<SerialPort>& arduino, std::string& content)
{
	return arduino->readSerialPort((uint8_t*)content.data(), content.length());
}


void write(std::unique_ptr<SerialPort>& arduino, const std::string& content)
{
	arduino->writeSerialPort((uint8_t*)content.c_str(), content.length());
}


using Arduino = std::unique_ptr<SerialPort>;

ISenderRef scan()
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
			return std::make_shared<SenderSerial>(std::move(arduino));
		}
	}

	return nullptr;
}

void printSerialInput(ISenderRef arduino)
{
	std::string str;
	str.resize(512);
	int i = arduino->ReceiveBuffer((void*)str.data(), str.length());
	str.resize(i);
	if (i)
	{
		fmt::print("received {} bytes:\n", str.length());
		fmt::print(fmt::fg(fmt::color::deep_sky_blue), str);
	}
}

int main()
{
	std::cout << "Hello World!\n";

	if (ISenderRef arduino = scan())
	{
		if (arduino->CanSend())
		{
			CommandFill fill;
			fill.R = 255;
			fill.G = 127;
			fill.B = 20;
			
			arduino->PushCommand(fill);
			arduino->PushCommand(CommandLedOff());
			Sleep(5000);
			arduino->PushCommand(CommandLedOn());
		}
			
		while(1)
		{
			printSerialInput(arduino);
			Sleep(500);
		}

	}


}

