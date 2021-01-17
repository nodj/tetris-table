// johan.duparc

#include "RemoteDisplay.h"

// https://fmt.dev/latest/index.html
// rgb terminal require HKCU/Console/VirtualTerminalLevel:DWORD=1
#include "fmt/color.h"
#include "fmt/format.h"

#include <chrono>
#include <thread>

int main()
{
	fmt::print("Hello World!\n");

	RemoteDisplay remote;

	if (remote.Connect())
	{
		remote.Fill({15,15,15});

		while (true)
		{
			remote.PrintSerialInput(); // todo in another thread
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(500ms);
		}
	}
}

