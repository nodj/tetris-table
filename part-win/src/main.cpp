// This file is part of the tetris-table project. Copyright (c) Johan Duparc.

#include "RemoteDisplay.h"

#include "shared/types.h"

// https://fmt.dev/latest/index.html
// rgb terminal require HKCU/Console/VirtualTerminalLevel:DWORD=1
#include "fmt/color.h"
#include "fmt/format.h"

#include "tc/TetrisHost.h"

#include <chrono>
#include <thread>


int main()
{
	fmt::print("Hello World!\n");

	RemoteDisplay remote;

	tc::TetrisHost host;

	auto sleepms = [&](uint32_t ms)
	{
		remote.PrintSerialInput(); // todo in another thread
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	};

	auto patern1 = [&]
	{
		int32_t w = 50;
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
	};

	auto patern2 = [&]
	{
		remote.FrameBegin();
		remote.Clear();
		remote.SetPixel(0, {255,   0,   0});
		remote.SetPixel(1, {  1, 255,   0});
		remote.SetPixel(2, {  2,   0, 255});
		remote.SetPixel(7, {  3,   0, 255});
		remote.SetPixel(8, {255, 255, 255});
		remote.SetPixel(9, {255,   0,   0});
		remote.FrameEnd();

		sleepms(2000);
	};

	auto patern3 = [&](int i)
	{
		int b = 16;
		while (b--)
			remote.SetPixel(3+b, {uint8_t(100 * (i>>b & 1)),  0,  10});
	};

	auto patern_binaryClock = [&](int count, int ms)
	{
		for (int i = 0; i < count; ++i)
		{
			remote.FrameBegin();
			if (i==0) remote.Clear();
			patern3(i);
			remote.FrameEnd();
			sleepms(ms);
		}
	};

	if (remote.Connect())
	{
		while (true)
		{
			patern_binaryClock(1 << 8, 50);
		}
	}
}
