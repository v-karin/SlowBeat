/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SlowBeatAudioProcessorEditor: public juce::AudioProcessorEditor, private juce::Timer {
public:
    SlowBeatAudioProcessorEditor(SlowBeatAudioProcessor &, juce::AudioProcessorValueTreeState &);
    ~SlowBeatAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

private:
    SlowBeatAudioProcessor &audioProcessor;

    juce::Slider loopLengthSlider;
    juce::Slider startLengthSlider;
    juce::Slider endLengthSlider;

    juce::Slider speedSlider;
    juce::Slider fadeDial;
    juce::Slider phaseDial;
    juce::Slider mixDial;

    juce::TextButton onButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> loopLengthAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> startLengthAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> endLengthAtt;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fadeAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> phaseAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAtt;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> onAtt;

    juce::Label bpmLabel;
    juce::Label ppqPositionLabel;
    juce::Label loopLengthLabel;
    juce::Label startLengthLabel;
    juce::Label endLengthLabel;

    juce::Label speedLabel;
    juce::Label mixLabel;

    juce::Label internalVolumeLabel;
    juce::Label fadeLabel;
    juce::Label phaseLabel;

    juce::Label headLabel;
    juce::Label head2Label;

    void timerCallback();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SlowBeatAudioProcessorEditor)
};
