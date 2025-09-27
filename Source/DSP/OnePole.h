#pragma once

class OnePole {
public:
    void prepare (double sampleRate) { fs = sampleRate; setHighpass (cutoff); }
    void setHighpass (float freq) {
        cutoff = freq;
        float x = std::exp (-2.0f * 3.1415926f * cutoff / (float) fs);
        a0 = (1.0f + x) * 0.5f; b1 = -x; z1 = 0.0f;
    }
    float process (float x) {
        float y = a0 * x + a0 * x1 + b1 * y1; // simple HP-ish using leaky differentiator idea
        x1 = x; y1 = y; return y;
    }
private:
    double fs = 48000.0;
    float cutoff = 4000.0f;
    float a0 = 0.5f, b1 = -0.5f; 
    float z1 = 0.0f, x1 = 0.0f, y1 = 0.0f;
};
