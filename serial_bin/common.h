// johan.duparc

#pragma once

#define WITH_LEDSTRIP0 1

#if WITH_LEDSTRIP0
    #define PIN       7
    #define NUMPIXELS 300
    #define DELAYVAL  0
#endif


struct Color_24b {
	uint8_t R = 127;
	uint8_t G = 0;
	uint8_t B = 0;
};
