// This file is part of the tetris-table project. Copyright (c) Johan Duparc.

#pragma once
#include <stdint.h>

using u8 = uint8_t;


struct Color_24b {
	Color_24b (uint8_t R=0, uint8_t G=0, uint8_t B=0): R(R), G(G), B(B) {}
	uint8_t R;
	uint8_t G;
	uint8_t B;
};

