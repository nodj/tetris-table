// This file is part of the tetris-table project. Copyright (c) Johan Duparc.

#pragma once

#include "shared/commands.h"
#include "shared/types.h"

#include <vector>



class OBuffer_ToVector : public IOBuffer
{
public:
	OBuffer_ToVector(std::vector<uint8_t>& buffer) : buffer(buffer) {}

private:
	virtual IOBuffer& ProcessOneByte(uint8_t& arg) override { buffer.push_back(arg); return *this; }
	std::vector<uint8_t>& buffer;
};
