/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SlowBeatAudioProcessor::SlowBeatAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)
    ),
#else
    :
#endif
    apvts(*this, nullptr, juce::Identifier("SlowBeatPlugin"), {
    std::make_unique<juce::AudioParameterInt>("loop_length", "Loop Length", 0, 15, 7),
    std::make_unique<juce::AudioParameterInt>("start_length", "Start Length", 0, 7, 1),
    std::make_unique<juce::AudioParameterInt>("end_length", "End Length", 0, 7, 1),
    std::make_unique<juce::AudioParameterInt>("speed", "Speed", 0, 11, 6),

    std::make_unique<juce::AudioParameterFloat>("fade", "Fade", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.0f, 0.5f, true), 0.05f),
    std::make_unique<juce::AudioParameterFloat>("phase", "Phase", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f), 0.0f),
    std::make_unique<juce::AudioParameterFloat>("mix", "Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f), 1.0f),

    std::make_unique<juce::AudioParameterBool>("on", "On", true)
    }) {
}

SlowBeatAudioProcessor::~SlowBeatAudioProcessor() {
}

//==============================================================================
const juce::String SlowBeatAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool SlowBeatAudioProcessor::acceptsMidi() const {
    return false;
}

bool SlowBeatAudioProcessor::producesMidi() const {
    return false;
}

bool SlowBeatAudioProcessor::isMidiEffect() const {
    return false;
}

double SlowBeatAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int SlowBeatAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int SlowBeatAudioProcessor::getCurrentProgram() {
    return 0;
}

void SlowBeatAudioProcessor::setCurrentProgram(int index) {
}

const juce::String SlowBeatAudioProcessor::getProgramName(int index) {
    return {};
}

void SlowBeatAudioProcessor::changeProgramName(int index, const juce::String &newName) {
}

//==============================================================================
void SlowBeatAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    filter.prepareToPlay();
}

void SlowBeatAudioProcessor::releaseResources() {
    filter.finishPlayback();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SlowBeatAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}
#endif

void SlowBeatAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();

    SlowBeatParams params;

    params.sampleRate = getSampleRate();

    // get playhead data
    auto *head = getPlayHead();
    if (head != nullptr) {
        auto info = head->getPosition();
        if (info != std::nullopt) {
            auto bpm = info->getBpm();
            if (bpm.hasValue()) {
                params.bpm = *bpm;
                params.hasParams |= FLAG_BPM;
            }

            auto ppqPosition = info->getPpqPosition();
            if (ppqPosition.hasValue()) {
                params.ppqPosition = *ppqPosition;
                params.hasParams |= FLAG_PPQ_POSITION;
            }

            params.isPlaying = info->getIsPlaying();
        }
    }
    
    params.loopLength = static_cast<char>(*apvts.getRawParameterValue("loop_length"));
    params.startLength = static_cast<char>(*apvts.getRawParameterValue("start_length"));
    params.endLength = static_cast<char>(*apvts.getRawParameterValue("end_length"));

    params.speed = static_cast<char>(*apvts.getRawParameterValue("speed"));
    params.fade = *apvts.getRawParameterValue("fade");
    params.phase = *apvts.getRawParameterValue("phase");
    params.mix = *apvts.getRawParameterValue("mix");

    params.on = (*apvts.getRawParameterValue("on") > 0.5) ? true : false;

    filter.prepareProcessBlock(numChannels, numSamples, params);

    for (int channel = 0; channel < numChannels; ++channel) {
        auto *channelData = buffer.getWritePointer(channel);

        filter.processChannel(channel, channelData);
    }

    filter.finishProcessBlock();
}

//==============================================================================
bool SlowBeatAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *SlowBeatAudioProcessor::createEditor() {
    return new SlowBeatAudioProcessorEditor(*this, apvts);
}

//==============================================================================
void SlowBeatAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SlowBeatAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

const SlowBeatReturn &SlowBeatAudioProcessor::getVariables() {
    return filter.getVariables();
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
    return new SlowBeatAudioProcessor();
}
