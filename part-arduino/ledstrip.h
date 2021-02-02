// johan.duparc

#pragma once

#include <Arduino.h>

#include "shared/config.h"

struct Color_24b;

class Strip
{
public:
    void Setup();
    void Apply();

    void Clear();
    void Fill(const Color_24b& color);
    void SetPixel(uint16_t index, const Color_24b& color);
};

#if WITH_LEDSTRIP0
extern Strip strip0;
#endif // WITH_LEDSTRIP0
