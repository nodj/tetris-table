// johan.duparc

#include "SerialPort.hpp"

// https://fmt.dev/latest/index.html
// rgb terminal require HKCU/Console/VirtualTerminalLevel:DWORD=1
#include "fmt/format.h"
#include "fmt/color.h"


#include <string>
#include <iostream>
#include "RemoteDisplay.h"
// 

int main()
{
	std::cout << "Hello World!\n";

	RemoteDisplay remote;

	if (remote.Connect())
	{
		remote.Fill({15,15,15});
	}

// 	if (ISenderRef arduino = scan())
// 	{
// 		if (arduino->CanSend())
// 		{
// 			CommandFill fill;
// 			fill.R = 255;
// 			fill.G = 127;
// 			fill.B = 20;
// 			
// 			arduino->PushCommand(fill);
// 			arduino->PushCommand(CommandLedOff());
// 			Sleep(5000);
// 			arduino->PushCommand(CommandLedOn());
// 		}
// 			
// 
// 	}

	while(1)
	{
		Sleep(500);
		remote.PrintSerialInput(); // todo in another thread
	}

}

