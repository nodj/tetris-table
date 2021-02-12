// This file is part of the tetris-table project. Copyright (c) Johan Duparc.

#include "SerialReaderHelper.h"
#include "commands.h"
#include "log.h"


SerialReaderHelper::~SerialReaderHelper()
{
	if (int consumed = availableOnCtr - available)
	{
		DeclareConsumedSerialByte(consumed);
	}
}

int16_t SerialReaderHelper::ReadAtMost(uint8_t* buffer, int16_t maxByteCount)
{
	const int16_t readCount = max(min(available, maxByteCount), 0);
	logV("RaM: a:"); logV(available); logV(" m:"); logV(maxByteCount); logV(" r:"); logVln(readCount);

	if (readCount)
	{
		available -= readCount;
		for (int i = 0; i < readCount; ++i)
			buffer[i] = (uint8_t)Serial.read();
	}
	return readCount;
}

