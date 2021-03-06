// This file is part of the tetris-table project. Copyright (c) Johan Duparc.

#pragma once

#include <Arduino.h>
#include <stdint.h>


struct SerialReaderHelper
{
	// todo: copy all in an internal buffer, send ack to win, and then parse.
	// -> allows sender to send the following bytes sooner
	// -> prevents interleaved direct serial read which would break the available byte count

	SerialReaderHelper()
		: available(Serial.available())
		, availableOnCtr(available)
	{
	}

	~SerialReaderHelper();

	SerialReaderHelper& operator >> (uint16_t& u16)
	{
		if (CanRead(2))
		{
			uint8_t lo = Serial.read();
			uint8_t hi = Serial.read();
			available -= 2;
			u16 = hi << 8 | lo;
		}
		else { parseOk = false; }
		return *this;
	}

	SerialReaderHelper& operator >> (uint8_t& u8)
	{
		if (CanRead(1))
		{
			u8 = Serial.read();
			available -= 1;
		}
		else { parseOk = false; }
		return *this;
	}

	bool CanRead(int16_t byteCount) const { return available >= byteCount; }
	int16_t Available() const { return available; }
	int Peek() const { return Serial.peek(); }
	int16_t ReadAtMost(uint8_t* buffer, int16_t maxByteCount);
	void SkipByte(int i) { while (i-- && available--) Serial.read(); }

private:
	int available;
	int availableOnCtr;
	bool parseOk = true;
};

