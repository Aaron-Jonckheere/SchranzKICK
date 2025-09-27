#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

class SchranzKickAudioProcessor;

class SchranzKickAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SchranzKickAudioProcessorEditor (SchranzKickAudioProcessor&);
    ~SchranzKickAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SchranzKickAudioProcessor& processorRef;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SchranzKickAudioProcessorEditor)
};
