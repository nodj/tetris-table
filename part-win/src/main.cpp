// johan.duparc

#include "RemoteDisplay.h"

#include "shared/types.h"

// https://fmt.dev/latest/index.html
// rgb terminal require HKCU/Console/VirtualTerminalLevel:DWORD=1
#include "fmt/color.h"
#include "fmt/format.h"

#include <chrono>
#include <thread>


// wip:
// 	-> RemoteDisplay handles the command api, simply Sync push .
// 	-> SenderSerial handle packet splitting (remote buffer < 64 bytes), acknowledgment, build Answers.


int main()
{
	fmt::print("Hello World!\n");

	RemoteDisplay remote;

	auto sleepms = [&](uint32_t ms)
	{
		remote.PrintSerialInput(); // todo in another thread
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	};

	if (remote.Connect())
	{
		int32_t w = 500;

		while (true)
		{
			u8 test = 10;
			remote.Fill({test,0,0});
			sleepms(w);
			remote.Fill({50,0,0});
			sleepms(w);
			remote.Fill({100,0,0});
			sleepms(w);
			remote.Fill({255,0,0});
			sleepms(w);
			remote.Fill({0,255,0});
			sleepms(w);
			remote.Fill({0,0,255});
			sleepms(w);
			remote.Fill({255,255,255});
			sleepms(w);
			remote.Clear();
			sleepms(w);
		}
	}
}

