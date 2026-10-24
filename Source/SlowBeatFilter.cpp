#include "SlowBeatFilter.h"



SlowBeatFilter::SlowBeatFilter() {
}

void SlowBeatFilter::prepareToPlay() {
}

void SlowBeatFilter::prepareProcessBlock(int inputChannels, int outputChannels, int numSamples, SlowBeatParams params) {
	auto oldSampleRate = mSampleRate;
	auto oldBpm = mBpm;

	mSampleRate = params.sampleRate;
	if (params.hasParams && FLAG_BPM) {
		mBpm = params.bpm;
	}

	if (oldSampleRate != mSampleRate || oldBpm != mBpm) {
		mBeatLengthSamples = (int)(mSampleRate / mBpm * 60.0);

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
	float mult = (loopLength & 0x01) ? 1.0f : 2.0f / 3.0f; // even = triplet
	mLoopLength = (float)(2 << (loopLength >> 1)) / 8.0f * mult;

	int fadeSamples = (int)(mFade * mBeatLengthSamples);
	int mBLSdiv4 = mBeatLengthSamples >> 2;

	auto startLength = params.startLength;
	mStartLength = (startLength == 0) ? 0 : (startLength == 1) ? fadeSamples : (2 << startLength) * mBLSdiv4;

	auto endLength = params.endLength;
	mEndLength = (endLength == 0) ? 0 : (endLength == 1) ? fadeSamples : (2 << endLength) * mBLSdiv4;

	mSpeed = (float)(12 - params.speed) / 12.0f;

	if (params.hasParams && FLAG_PPQ_POSITION) {
		auto prevModLoop = fmod(mPpqPosition, mLoopLength);
		auto prevModFull = fmod(mPpqPosition, 32.0f);

		mPpqPosition = (float)params.ppqPosition;

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