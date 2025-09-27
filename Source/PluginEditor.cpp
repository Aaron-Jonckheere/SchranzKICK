#include "PluginEditor.h"
#include "PluginProcessor.h"

using namespace juce;

SchranzKickAudioProcessorEditor::SchranzKickAudioProcessorEditor (SchranzKickAudioProcessor& p)
: AudioProcessorEditor (&p), processorRef (p)
{
    setSize (620, 420);
}

void SchranzKickAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::black);
    g.setColour (Colours::white);
    g.setFont (Font (18.0f));
    g.drawFittedText ("SchranzKick — gebruik Generic Editor om parameters te tweaken", getLocalBounds(), Justification::centred, 2);
}

void SchranzKickAudioProcessorEditor::resized() {}
