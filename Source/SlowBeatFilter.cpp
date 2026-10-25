#include "SlowBeatFilter.h"



static float loopLengthBars(int exponent, int split) {
	auto mult = 2.0f / static_cast<float>(split);
	return static_cast<float>(2 << exponent) / 8.0f * mult;
}

static int sampleBorderLength(int exponent, int fadeSamples, int lengthMult) {
	return exponent == 0 ? 0 : (exponent == 1 ? fadeSamples : (2 << exponent) * lengthMult);
}

SlowBeatFilter::SlowBeatFilter() {
}

void SlowBeatFilter::prepareToPlay() {
}

void SlowBeatFilter::prepareProcessBlock(int inputChannels, int outputChannels, int numSamples, SlowBeatParams &params) {
	auto oldSampleRate = mSampleRate;
	auto oldBpm = mBpm;

	mSampleRate = params.sampleRate;
	if (params.hasParams && FLAG_BPM) {
		mBpm = params.bpm;
	}

	if (oldSampleRate != mSampleRate || oldBpm != mBpm) {
		mBeatLengthSamples = static_cast<int>(mSampleRate / mBpm * 60.0);

		int newSampleSize = (mBeatLengthSamples + 1) << 5;
		mInternalBuffer.setSize(outputChannels, newSampleSize, false, true, false);

		mInterpolators.resize(outputChannels);
		for (auto &interpolator : mInterpolators) {
			interpolator.reset();
		}

		mHead = 0;
		mHead2 = 0;
	}

	mNumSamples = numSamples;
	mOn = params.on;
	mMix = params.mix;
	mFade = params.fade;

	auto loopLength = params.loopLength;
	int split = (loopLength & 1) ? 2 : 3; // even = triplet
	mLoopLength = loopLengthBars(static_cast<int>(loopLength >> 1), split);

	int fadeSamples = static_cast<int>(mFade * mBeatLengthSamples);
	mStartLength = sampleBorderLength(static_cast<int>(params.startLength), fadeSamples, mBeatLengthSamples / 4);
	mEndLength = sampleBorderLength(static_cast<int>(params.endLength), fadeSamples, mBeatLengthSamples / 4);

	mSpeed = static_cast<float>(12 - params.speed) / 12.0f;

	if (params.hasParams && FLAG_PPQ_POSITION) {
		auto prevModLoop = fmod(mPpqPosition, mLoopLength);
		auto prevModFull = fmod(mPpqPosition, 32.0f);

		mPpqPosition = static_cast<float>(params.ppqPosition);

		if (prevModFull > fmod(mPpqPosition, 32.0f) || mHead > mInternalBuffer.getNumSamples() || !params.isPlaying || mPpqPosition <= std::numeric_limits<float>::epsilon()) {
			mHead = 0;
		}

		if (prevModLoop > fmod(mPpqPosition, mLoopLength) || mHead2 > mHead) {
			mHead2 = mHead;
		}
	}
}

void SlowBeatFilter::processChannel(int channel, float *channelData) {
	if (mOn) {
		float *internalChannelData = mInternalBuffer.getWritePointer(channel, mHead);
		for (int i = 0; i < mNumSamples; ++i) {
			internalChannelData[i] = channelData[i];
		}
		mProcessed = mInterpolators[channel].process(mSpeed, mInternalBuffer.getWritePointer(channel, mHead2), channelData, mNumSamples);
	}
}

void SlowBeatFilter::finishProcessBlock() {
	mHead += mNumSamples;
	mHead2 += mProcessed;
}

void SlowBeatFilter::finishPlayback() {
	mInternalBuffer.setSize(0, 0);

	mHead = 0;
	mHead2 = 0;
}

SlowBeatReturn SlowBeatFilter::getVariables() {
	SlowBeatReturn variables;

	variables.bpm = mBpm;
	variables.ppqPosition = mPpqPosition;
	variables.loopLength = mLoopLength;
	variables.startLength = mStartLength;
	variables.endLength = mEndLength;
	variables.speed = mSpeed;
	variables.mix = mMix;
	variables.internalVolume = mInternalVolume;
	variables.fade = mFade;
	variables.phase = mPhase;
	variables.head = mHead;
	variables.head2 = mHead2;

	return variables;
}