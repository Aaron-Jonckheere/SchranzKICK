#pragma once
#include <juce_dsp/juce_dsp.h>
#include "OnePole.h"
#include "Distortion.h"

struct KickParams
{
    float baseHz = 50.f;
    float startRatio = 8.f;
    float pitchDecayMs = 80.f;
    float pitchCurve = 1.0f; // expo curve factor
    bool tuneFromMidi = true;

    float lengthMs = 250.f;
    float ampDecayMs = 200.f;
    float harmonics = 0.2f;
    float shape = 0.0f;

    float click = 0.4f;
    float clickLenMs = 5.f;
    float clickHPF = 4000.f;

    float drive1 = 12.f;
    float drive2 = 6.f;
    int   bits   = 24;

    bool limiter = true;
};

class KickVoice
{
public:
    void prepare (double sampleRate)
    {
        fs = sampleRate;
        reset();
        clickHPF.prepare (fs);
    }

    void reset()
    {
        phase = 0.0; time = 0.0; gate = false;
        ampEnv = 0.0f; clickEnv = 0.0f; 
        rng.setSeedRandomly();
    }

    void noteOn (int midiNote, float vel)
    {
        lastNote = midiNote; velocity = vel; gate = true; time = 0.0; phase = 0.0; ampEnv = 1.0f; clickEnv = 1.0f;
    }

    void noteOff() { gate = false; }

    void updateParameters (juce::AudioProcessorValueTreeState& s)
    {
        params.tuneFromMidi = ((int)*s.getRawParameterValue ("TUNE_MODE") == 0);
        params.baseHz = *s.getRawParameterValue ("BASE_HZ");
        params.startRatio = *s.getRawParameterValue ("START_RATIO");
        params.pitchDecayMs = *s.getRawParameterValue ("PITCH_DECAY");
        params.pitchCurve = *s.getRawParameterValue ("PITCH_CURVE");
        params.lengthMs = *s.getRawParameterValue ("LENGTH");
        params.ampDecayMs = *s.getRawParameterValue ("AMP_DECAY");
        params.harmonics = *s.getRawParameterValue ("HARM");
        params.shape = *s.getRawParameterValue ("SHAPE");
        params.click = *s.getRawParameterValue ("CLICK");
        params.clickLenMs = *s.getRawParameterValue ("CLICK_LEN");
        params.clickHPF = *s.getRawParameterValue ("CLICK_HPF");
        params.drive1 = *s.getRawParameterValue ("DRIVE1");
        params.drive2 = *s.getRawParameterValue ("DRIVE2");
        params.bits = (int)*s.getRawParameterValue ("BITS");
        params.limiter = *s.getRawParameterValue ("LIM") > 0.5f;
        clickHPF.setHighpass (params.clickHPF);
    }

    void renderNextBlock (float* out, int n)
    {
        for (int i = 0; i < n; ++i)
        {
            float s = processSample();
            out[i] = s;
        }
    }

private:
    double fs = 48000.0;
    double phase = 0.0;
    double time = 0.0;
    bool gate = false;
    int lastNote = 36;
    float velocity = 1.0f;
    float ampEnv = 0.0f;
    float clickEnv = 0.0f;
    KickParams params;

    OnePole clickHPF;

    juce::Random rng;

    inline float midiToHz (int note) const { return 440.0f * std::pow (2.0f, (note - 69) / 12.0f); }

    float processSample()
    {
        // Time step
        const float dt = 1.0f / (float) fs;
        time += dt;

        // Lifetime
        float lengthSec = params.lengthMs * 0.001f;
        if (time > lengthSec) gate = false;

        // Pitch envelope
        float baseHz = params.tuneFromMidi ? midiToHz (lastNote) : params.baseHz;
        float startHz = baseHz * params.startRatio;
        float pitchT = std::min (1.0f, (float)(time / (params.pitchDecayMs * 0.001f)));
        // Exponential like curve control
        float curve = params.pitchCurve;
        float env = std::pow (1.0f - pitchT, curve);
        float currHz = juce::jlimit (10.0f, 4000.0f, baseHz + (startHz - baseHz) * env);

        // Oscillator (sine + gentle harmonics)
        phase += (2.0 * juce::MathConstants<double>::pi * currHz) / fs;
        if (phase > 2.0 * juce::MathConstants<double>::pi)
            phase -= 2.0 * juce::MathConstants<double>::pi;

        float sine = std::sin ((float)phase);
        float third = std::sin (3.0f * (float)phase) * params.harmonics * 0.3f;
        float body = juce::jlimit (-1.0f, 1.0f, (1.0f - params.shape) * sine + params.shape * (0.7f * std::tanh (2.5f * sine)) + third);

        // Amplitude envelope (simple exp decay)
        float ampCoeff = std::exp (-1.0f / std::max (1.0f, (float)fs * (params.ampDecayMs * 0.001f)));
        ampEnv = gate ? std::max (std::exp (- (float)time / (params.ampDecayMs * 0.001f)), 0.0f) : ampEnv * ampCoeff;

        // Click layer (noise burst with HPF)
        float clickT = std::min (1.0f, (float)(time / (params.clickLenMs * 0.001f)));
        clickEnv = 1.0f - clickT;
        float noise = (rng.nextFloat() * 2.0f - 1.0f);
        float click = clickHPF.process (noise) * clickEnv * params.click;

        float s = body * ampEnv + click;

        // Distortion stages
        s = Distortion::tanhDrive (s, params.drive1);
        s = Distortion::hardClip (s, params.drive2);
        s = Distortion::bitcrush (s, params.bits);

        // Output soft limiting to avoid crazy peaks
        if (params.limiter)
            s = juce::jlimit (-0.98f, 0.98f, s);

        return s * velocity;
    }
};
