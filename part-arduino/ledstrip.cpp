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
	// as the documentation says, this call interfere with interupts.
	// Hence, it messes with the Serial bus and causes data loss...

	pixels.show();
}

void Strip::Clear()
{
	pixels.clear();
}

void Strip::Fill(const Color_24b& color)
{
	pixels.fill(Adafruit_NeoPixel::Color(color.R, color.G, color.B));
}

void Strip::SetPixel(uint16_t index, const Color_24b& color)
{
	pixels.setPixelColor(index, color.R, color.G, color.B);
}
