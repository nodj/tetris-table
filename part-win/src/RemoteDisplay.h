// johan.duparc

#pragma once

#include "SenderInterface.h"

#include <memory>


struct Color_24b { 
	uint8_t R = 127;
	uint8_t G = 0;
	uint8_t B = 0;
};


class RemoteDisplay
{
public:
	bool Connect();

	bool IsConnected();

	void Clear(){ Fill({0,0,0}); }
	
	void Fill(const Color_24b& fillColor);

	void PrintSerialInput();
	
private:
	std::unique_ptr<class SenderInterface> sender;
};