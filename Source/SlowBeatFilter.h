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

	char hasParams{ '\0' };
	bool on;
	bool isPlaying;
};

const char FLAG_BPM{ '\x01'};
const char FLAG_PPQ_POSITION{ '\x02'};

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
	int mChannels{ 2 };
};




class SampleBlockHead {
public:
	SampleBlockHead(int headPosition, int lengthSamples, int fadeSamples, float speed, InterpolatorPool &pool);
	~SampleBlockHead();
	void writeSamples(const juce::AudioBuffer<float> &input, juce::AudioBuffer<float> &output, int numSamples);
	void forceFade();

private:
	void checkBuffer(int numSamples);
	int writeChannels(const juce::AudioBuffer<float> &input, juce::AudioBuffer<float> &output, int numSamples, float startGain, float endGain);
	juce::AudioBuffer<float> mBuffer;
	InterpolatorPool &mAssignedPool;
	int mInputHead;
	int mRemainingLength;
	int mFadeLength;
	int mIdx;
	float mSpeed;

	float mWeight{ 0.0f };
};




class SlowBeatFilter {
public:
	SlowBeatFilter();
	
	void prepareToPlay();

	void prepareProcessBlock(int numChannels, int numSamples, SlowBeatParams &params);
	void processBlock(juce::AudioBuffer<float> &buffer, SlowBeatParams &params);
	void finishProcessBlock();

	void finishPlayback();

	const SlowBeatReturn getVariables();

private:
	float loopLengthBars(int exponent, int split);
	int sampleBorderLength(int exponent, int fadeSamples, int lengthMult);
	void processChannel(int channel, float *channelData);

	juce::AudioBuffer<float> mInternalBuffer;
	std::vector<juce::Interpolators::CatmullRom> mInterpolators;

	double mSampleRate{ 0.0 };
	double mBpm{ 0.0 };
	double mBpmInv{ 0.0 };

	float mPpqPosition{ 0.0f };
	float mLoopLength{ 0.0f };

	float mInternalVolume{ 0.0f };
	float mInternalVolumeIncrement{ 0.0f };

	float mSpeed{ 1.0f };
	float mMix{ 1.0f };
	float mFade{ 0.0f };
	float mPhase{ 0.0f };

	int mChannels{ 0 };
	int mNumSamples{ 0 };
	int mStartLength{ 0 };
	int mEndLength{ 0 };

	int mInputHead{ 0 };
	int mOutputHead{ 0 };
	int mBeatLengthSamples{ 0 };
	int mProcessed{ 0 };

	bool mOn{ false };
	bool preparingToPlay{ false };
};
