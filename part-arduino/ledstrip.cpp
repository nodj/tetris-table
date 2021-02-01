// johan.duparc

#include "ledstrip.h"

#include "Adafruit_NeoPixel.h"
#include "shared/types.h"


#if WITH_LEDSTRIP0
Strip strip0;
#endif // WITH_LEDSTRIP0

//led strip: SK6812 RGBW (BTF-5V-060L-W) 90W

// a static Adafruit_NeoPixel object obv doesn't scale with more than 1 strip. It's quite stupid.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRBW + NEO_KHZ800);


void Strip::Setup()
{
    pixels.begin();
}

void Strip::Apply()
{
    pixels.show();
}

void Strip::Clear()
{
    pixels.clear();
}

void Strip::Fill(const Color_24b& color)
{
    uint32_t c = 0;
    uint32_t w = 0;
    c = w << 24 | uint32_t(color.R) << 16 | uint32_t(color.G) << 8 | uint32_t(color.B);
    pixels.fill(c);
}