#pragma once

#include <vector>
#include "JuceHeader.h"




struct SlowBeatParams {
	double sampleRate;

	double bpm;
	double ppqPosition;

	float fade;
	float phase;
	float mix;

	char loopLength;
	char startLength;
	char endLength;
	char speed;

	char hasParams = '\0';
	bool on;
	bool isPlaying;
};

const char FLAG_BPM = 0x01;
const char FLAG_PPQ_POSITION = 0x02;

struct SlowBeatReturn {
	double bpm;
	float ppqPosition;
	float loopLength;
	int startLength;
	int endLength;

	float speed;
	float mix;

	float internalVolume;
	float fade;
	float phase;

	int inputHead;
	int outputHead;
};




class InterpolatorPool {
public:
	InterpolatorPool();
	void setChannels(int channels);
	int getFreeInterpolatorIdx();
	void releaseInterpolatorIdx(int idx);
	int processChannel(int idx, int channel, double speed, const float *input, float *output, int amount);

private:
	std::vector<std::vector<juce::Interpolators::CatmullRom>> mInterpolators;
	std::vector<bool> mOccupied;
	std::vector<bool> mReady;
	int mChannels = 2;
};




class SlowBeatFilter {
public:
	SlowBeatFilter();
	
	void prepareToPlay();

	void prepareProcessBlock(int inputChannels, int outputChannels, int numSamples, SlowBeatParams &params);
	void processChannel(int channel, float *channelData);
	void finishProcessBlock();

	void finishPlayback();

	const SlowBeatReturn &getVariables();

private:
	//std::vector<std::vector<float>> mInternalBuffer;
	juce::AudioBuffer<float> mInternalBuffer;
	std::vector<juce::Interpolators::CatmullRom> mInterpolators;

	double mSampleRate = 0.0;
	double mBpm = 0.0;
	double mBpmInv = 0.0;

	float mPpqPosition = 0.0;
	float mLoopLength = 0;

	float mInternalVolume = 0.0;
	float mInternalVolumeIncrement = 0.0;

	float mSpeed = 1.0;
	float mMix = 1.0;
	float mFade = 0.0;
	float mPhase = 0.0;

	int mOutputChannels = 0;
	int mNumSamples = 0;
	int mStartLength = 0;
	int mEndLength = 0;

	int mInputHead = 0;
	int mOutputHead = 0;
	int mBeatLengthSamples = 0;
	int mProcessed = 0;

	bool mOn = false;
	bool preparingToPlay = false;
};
