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




class SlowBeatFilter {
public:
	SlowBeatFilter();
	
	void prepareToPlay();

	void prepareProcessBlock(int, int, SlowBeatParams &);
	void processBlock(juce::AudioBuffer<float> &, SlowBeatParams &);
	void finishProcessBlock();

	void finishPlayback();

	const SlowBeatReturn getVariables();

private:
	float loopLengthBars(int, int);
	int sampleBorderLength(int, int, int);
	void processChannel(int, float *);

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
