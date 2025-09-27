#pragma once
#include <cmath>

namespace Distortion {
    inline float tanhDrive (float x, float driveDb)
    {
        float g = std::pow (10.0f, driveDb / 20.0f);
        return std::tanh (x * g);
    }

    inline float hardClip (float x, float driveDb)
    {
        float g = std::pow (10.0f, driveDb / 20.0f);
        float y = x * g;
        if (y > 1.0f) y = 1.0f; else if (y < -1.0f) y = -1.0f;
        return y;
    }

    inline float bitcrush (float x, int bits)
    {
        bits = std::max (4, std::min (24, bits));
        const float steps = (float)((1u << (bits - 1)) - 1u);
        return std::round (x * steps) / steps;
    }
}
