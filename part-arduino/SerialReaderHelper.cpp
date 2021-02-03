// johan.duparc

#include "SerialReaderHelper.h"
#include "commands.h"

SerialReaderHelper::~SerialReaderHelper()
{
	if (int consumed = availableOnCtr - available)
	{
		DeclareConsumedSerialByte(consumed);
	}
}

int16_t SerialReaderHelper::ReadAtMost(uint8_t* buffer, int16_t maxByteCount)
{
	Serial.print("a:");
	Serial.println(available);
	const int16_t readCount = max(min(available, maxByteCount), 0);
	if (readCount)
	{
		available -= readCount;
		for (int i = 0; i < readCount; ++i)
			buffer[i] = (uint8_t)Serial.read();
	}
	return readCount;
}

