#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

SchranzKickAudioProcessor::SchranzKickAudioProcessor()
: AudioProcessor (BusesProperties().withOutput ("Out", AudioChannelSet::stereo(), true))
{
}

void SchranzKickAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    voice.prepare (sampleRate);

    auto* osParam = apvts.getRawParameterValue("OS");
    oversampleFactor = (*osParam > 0.5f ? 2 : 1);

    // Oversampling stage 2x (if enabled)
    std::get<0>(postChain).reset();
    std::get<0>(postChain) = dsp::Oversampling<float> (2 /* channels */, oversampleFactor == 2 ? 1 : 0, dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);

    dsp::ProcessSpec spec { sampleRate * (double)oversampleFactor, (uint32) samplesPerBlock, 2 };
    std::get<0>(postChain).initProcessing ((size_t) samplesPerBlock);
    std::get<1>(postChain).prepare (spec);
    std::get<2>(postChain).prepare (spec);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SchranzKickAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& main = layouts.getMainOutputChannelSet();
    return main == AudioChannelSet::mono() || main == AudioChannelSet::stereo();
}
#endif

void SchranzKickAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();

    // Handle MIDI (monophonic): trigger on any note-on
    MidiMessage m; int pos;
    for (MidiBuffer::Iterator it { midi }; it.getNextEvent (m, pos); )
    {
        if (m.isNoteOn())  voice.noteOn (m.getNoteNumber(), m.getFloatVelocity());
        if (m.isNoteOff()) voice.noteOff();
    }

    // Generate kick into a temp mono buffer
    AudioBuffer<float> mono (1, numSamples);
    mono.clear();

    auto& params = apvts;
    voice.updateParameters (params);

    auto* out = mono.getWritePointer (0);
    voice.renderNextBlock (out, numSamples);

    // Post FX: oversampling + EQ
    dsp::AudioBlock<float> block (mono);
    dsp::ProcessContextReplacing<float> ctx (block);

    if (oversampleFactor == 2)
        block = std::get<0>(postChain).processSamplesUp (block);

    // Update EQ coeffs from params
    auto lowFreq  = *apvts.getRawParameterValue ("EQ_LO_FREQ");
    auto lowGain  = Decibels::decibelsToGain (*apvts.getRawParameterValue ("EQ_LO_GAIN"));
    auto pkFreq   = *apvts.getRawParameterValue ("EQ_PK_FREQ");
    auto pkQ      = *apvts.getRawParameterValue ("EQ_PK_Q");
    auto pkGainDb = *apvts.getRawParameterValue ("EQ_PK_GAIN");

    auto osRate = getSampleRate() * (double) oversampleFactor;
    *std::get<1>(postChain).state = *dsp::IIR::Coefficients<float>::makeLowShelf (osRate, lowFreq, 0.707f, lowGain);
    *std::get<2>(postChain).state = *dsp::IIR::Coefficients<float>::makePeakFilter (osRate, pkFreq, pkQ, Decibels::decibelsToGain (pkGainDb));

    std::get<1>(postChain).process (ctx);
    std::get<2>(postChain).process (ctx);

    if (oversampleFactor == 2)
        block = std::get<0>(postChain).processSamplesDown (block);

    // Output gain and limiter
    auto outGain = Decibels::decibelsToGain (*apvts.getRawParameterValue ("OUT"));

    auto* L = buffer.getWritePointer (0);
    auto* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        float s = std::clamp (mono.getSample (0, i) * outGain, -0.98f, 0.98f);
        L[i] = s;
        if (R) R[i] = s;
    }
}

void SchranzKickAudioProcessor::getStateInformation (MemoryBlock& dest)
{
    MemoryOutputStream mos (dest, false);
    apvts.state.writeToStream (mos);
}

void SchranzKickAudioProcessor::setStateInformation (const void* data, int size)
{
    ValueTree tree = ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

AudioProcessorEditor* SchranzKickAudioProcessor::createEditor() { return new GenericAudioProcessorEditor (*this); }

AudioProcessorValueTreeState::ParameterLayout SchranzKickAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    auto hzRange = NormalisableRange<float> (20.f, 2000.f, 0.01f, 0.3f);

    p.emplace_back (std::make_unique<AudioParameterChoice> ("TUNE_MODE", "Tune Mode", StringArray { "MIDI", "Fixed" }, 0));
    p.emplace_back (std::make_unique<AudioParameterFloat> ("BASE_HZ",  "Base Hz", hzRange, 50.f));
    p.emplace_back (std::make_unique<AudioParameterFloat> ("START_RATIO", "Start Ratio", NormalisableRange<float>(1.f, 16.f, 0.01f, 0.4f), 8.f));
    p.emplace_back (std::make_unique<AudioParameterFloat> ("PITCH_DECAY", "Pitch Decay (ms)", NormalisableRange<float>(5.f, 400.f, 0.1f, 0.5f), 80.f));
    p.emplace_back (std::make_unique<AudioParameterFloat> ("PITCH_CURVE", "Pitch Curve", NormalisableRange<float>(0.1f, 4.0f, 0.001f, 0.5f), 1.0f));
    p.emplace_back (std::make_unique<AudioParameterFloat> ("LENGTH", "Length (ms)", NormalisableRange<float>(10.f, 2000.f, 0.1f, 0.5f), 250.f));
    p.emplace_back (std::make_unique<AudioParameterFloat> ("AMP_DECAY", "Amp Decay (ms)", NormalisableRange<float>(5.f, 2000.f, 0.1f, 0.5f), 200.f));
    p.emplace_back (std::make_unique<AudioParameterFloat> ("HARM", "Harmonics", NormalisableRange<float>(0.f, 1.f, 0.0001f), 0.2f));
    p.emplace_back (std::make_unique<AudioParameterFloat> ("SHAPE", "Shape", NormalisableRange<float>(0.f, 1.f, 0.0001f), 0.0f));

    p.emplace_back (std::make_unique<AudioParameterFloat> ("CLICK", "Click Level", NormalisableRange<float>(0.f, 1.f, 0.0001f), 0.4f));
    p.emplace_back (std::make_unique<AudioParameterFloat> ("CLICK_LEN", "Click Length (ms)", NormalisableRange<float>(0.5f, 30.f, 0.01f, 0.4f), 5.f));
    p.emplace_back (std::make_unique<AudioParameterFloat> ("CLICK_HPF", "Click HPF (Hz)", NormalisableRange<float>(1000.f, 12000.f, 0.01f, 0.4f), 4000.f));

    p.emplace_back (std::make_unique<AudioParameterFloat> ("DRIVE1", "Drive 1", NormalisableRange<float>(0.f, 24.f, 0.01f), 12.f));
    p.emplace_back (std::make_unique<AudioParameterFloat> ("DRIVE2", "Drive 2", NormalisableRange<float>(0.f, 24.f, 0.01f), 6.f));
    p.emplace_back (std::make_unique<AudioParameterInt>   ("BITS",   "Bit Depth", 4, 24, 24));

    p.emplace_back (std::make_unique<AudioParameterFloat> ("EQ_LO_FREQ",  "LowShelf Freq", hzRange, 80.f));
    p.emplace_back (std::make_unique<AudioParameterFloat> ("EQ_LO_GAIN",  "LowShelf Gain (dB)", NormalisableRange<float>(-12.f, 12.f, 0.01f), 0.f));
    p.emplace_back (std::make_unique<AudioParameterFloat> ("EQ_PK_FREQ",  "Peak Freq", hzRange, 1500.f));
    p.emplace_back (std::make_unique<AudioParameterFloat> ("EQ_PK_Q",     "Peak Q", NormalisableRange<float>(0.1f, 10.f, 0.001f, 0.5f), 1.0f));
    p.emplace_back (std::make_unique<AudioParameterFloat> ("EQ_PK_GAIN",  "Peak Gain (dB)", NormalisableRange<float>(-18.f, 18.f, 0.01f), 0.f));

    p.emplace_back (std::make_unique<AudioParameterBool>  ("OS",  "Oversampling 2x", true));
    p.emplace_back (std::make_unique<AudioParameterBool>  ("LIM", "Limiter", true));
    p.emplace_back (std::make_unique<AudioParameterFloat> ("OUT", "Output (dB)", NormalisableRange<float>(-24.f, 12.f, 0.01f), -2.f));

    return { p.begin(), p.end() };
}
