// johan.duparc

#pragma once

#include "SenderInterface.h"
#include "shared/types.h"

#include <memory>


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