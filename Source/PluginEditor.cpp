/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SlowBeatAudioProcessorEditor::SlowBeatAudioProcessorEditor(SlowBeatAudioProcessor &p, juce::AudioProcessorValueTreeState &apvts)
    : AudioProcessorEditor(&p), audioProcessor(p) {

    loopLengthSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
    loopLengthSlider.setRange(0, 15, 7);
    addAndMakeVisible(loopLengthSlider);
    loopLengthAtt.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, "loop_length", loopLengthSlider));

    startLengthSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
    startLengthSlider.setRange(0, 7, 1);
    addAndMakeVisible(startLengthSlider);
    startLengthAtt.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, "start_length", startLengthSlider));

    endLengthSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
    endLengthSlider.setRange(0, 7, 1);
    addAndMakeVisible(endLengthSlider);
    endLengthAtt.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, "end_length", endLengthSlider));



    speedSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
    speedSlider.setRange(0, 11, 6);
    addAndMakeVisible(speedSlider);
    speedAtt.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, "speed", speedSlider));

    fadeDial.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    fadeDial.setRange(-1.0, 1.0, 0.0);
    addAndMakeVisible(fadeDial);
    fadeAtt.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, "fade", fadeDial));

    phaseDial.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    phaseDial.setRange(-1.0, 1.0, 0.0);
    addAndMakeVisible(phaseDial);
    phaseAtt.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, "phase", phaseDial));

    mixDial.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mixDial.setRange(-1.0, 1.0, 0.0);
    addAndMakeVisible(mixDial);
    mixAtt.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, "mix", mixDial));

    onButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    onButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
    onButton.setClickingTogglesState(true);
    addAndMakeVisible(onButton);
    onAtt.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(apvts, "on", onButton));

    addAndMakeVisible(bpmLabel);
    addAndMakeVisible(ppqPositionLabel);
    addAndMakeVisible(loopLengthLabel);
    addAndMakeVisible(startLengthLabel);
    addAndMakeVisible(endLengthLabel);

    addAndMakeVisible(speedLabel);
    addAndMakeVisible(mixLabel);

    addAndMakeVisible(internalVolumeLabel);
    addAndMakeVisible(fadeLabel);
    addAndMakeVisible(phaseLabel);

    addAndMakeVisible(headLabel);
    addAndMakeVisible(head2Label);

    startTimerHz(24);
    setSize(400, 400);
}

SlowBeatAudioProcessorEditor::~SlowBeatAudioProcessorEditor() {
}

//==============================================================================
void SlowBeatAudioProcessorEditor::paint(juce::Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
}

void SlowBeatAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();

    loopLengthSlider.setBounds(bounds.removeFromTop(50).withSizeKeepingCentre(400, 50));
    startLengthSlider.setBounds(bounds.removeFromTop(50).withSizeKeepingCentre(400, 50));
    endLengthSlider.setBounds(bounds.removeFromTop(50).withSizeKeepingCentre(400, 50));
    speedSlider.setBounds(bounds.removeFromTop(50).withSizeKeepingCentre(400, 50));

    fadeDial.setBounds(bounds.removeFromLeft(100).withSize(100, 100));
    phaseDial.setBounds(bounds.removeFromLeft(100).withSize(100, 100));
    mixDial.setBounds(bounds.removeFromLeft(100).withSize(100, 100));
    onButton.setBounds(bounds.removeFromLeft(100).withSize(100, 100));

    bounds.expand(400, 0);
    bounds.removeFromTop(100);

    bpmLabel.setBounds(bounds.removeFromLeft(100).withSize(100, 30));
    ppqPositionLabel.setBounds(bounds.removeFromLeft(100).withSize(100, 30));
    loopLengthLabel.setBounds(bounds.removeFromLeft(100).withSize(100, 30));
    startLengthLabel.setBounds(bounds.removeFromLeft(100).withSize(100, 30));

    bounds.expand(400, 0);
    bounds.removeFromTop(30);

    endLengthLabel.setBounds(bounds.removeFromLeft(100).withSize(100, 30));
    speedLabel.setBounds(bounds.removeFromLeft(100).withSize(100, 30));
    mixLabel.setBounds(bounds.removeFromLeft(100).withSize(100, 30));
    internalVolumeLabel.setBounds(bounds.removeFromLeft(100).withSize(100, 30));

    bounds.expand(400, 0);
    bounds.removeFromTop(30);

    fadeLabel.setBounds(bounds.removeFromLeft(100).withSize(100, 30));
    phaseLabel.setBounds(bounds.removeFromLeft(100).withSize(100, 30));
    headLabel.setBounds(bounds.removeFromLeft(100).withSize(100, 30));
    head2Label.setBounds(bounds.removeFromLeft(100).withSize(100, 30));
}

void SlowBeatAudioProcessorEditor::timerCallback() {
    const SlowBeatReturn &variables = audioProcessor.getVariables();

    bpmLabel.setText("BPM: " + std::to_string(variables.bpm), juce::NotificationType::dontSendNotification);
    ppqPositionLabel.setText("PPQ Pos: " + std::to_string(variables.ppqPosition), juce::NotificationType::dontSendNotification);
    loopLengthLabel.setText("Loop: " + std::to_string(variables.loopLength), juce::NotificationType::dontSendNotification);
    startLengthLabel.setText("Start: " + std::to_string(variables.startLength), juce::NotificationType::dontSendNotification);

    endLengthLabel.setText("End: " + std::to_string(variables.endLength), juce::NotificationType::dontSendNotification);
    speedLabel.setText("Speed: " + std::to_string(variables.speed), juce::NotificationType::dontSendNotification);
    mixLabel.setText("Mix: " + std::to_string(variables.mix), juce::NotificationType::dontSendNotification);
    internalVolumeLabel.setText("Int Vol: " + std::to_string(variables.internalVolume), juce::NotificationType::dontSendNotification);

    fadeLabel.setText("Fade: " + std::to_string(variables.fade), juce::NotificationType::dontSendNotification);
    phaseLabel.setText("Phase: " + std::to_string(variables.phase), juce::NotificationType::dontSendNotification);
    headLabel.setText("Head: " + std::to_string(variables.head), juce::NotificationType::dontSendNotification);
    head2Label.setText("Head: " + std::to_string(variables.head2), juce::NotificationType::dontSendNotification);
}