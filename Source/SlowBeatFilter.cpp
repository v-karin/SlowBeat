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

		mInputHead = 0;
		mOutputHead = 0;
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

		if (prevModFull > fmod(mPpqPosition, 32.0f) || mInputHead > mInternalBuffer.getNumSamples() || !params.isPlaying || mPpqPosition <= std::numeric_limits<float>::epsilon()) {
			mInputHead = 0;
		}

		if (prevModLoop > fmod(mPpqPosition, mLoopLength) || mOutputHead > mInputHead) {
			mOutputHead = mInputHead;
		}
	}
}

void SlowBeatFilter::processChannel(int channel, float *channelData) {
	if (mOn) {
		float *internalChannelData = mInternalBuffer.getWritePointer(channel, mInputHead);
		for (int i = 0; i < mNumSamples; ++i) {
			internalChannelData[i] = channelData[i];
		}
		mProcessed = mInterpolators[channel].process(mSpeed, mInternalBuffer.getWritePointer(channel, mOutputHead), channelData, mNumSamples);
	}
}

void SlowBeatFilter::finishProcessBlock() {
	mInputHead += mNumSamples;
	mOutputHead += mProcessed;
}

void SlowBeatFilter::finishPlayback() {
	mInternalBuffer.setSize(0, 0);

	mInputHead = 0;
	mOutputHead = 0;
}

const SlowBeatReturn &SlowBeatFilter::getVariables() {
	const SlowBeatReturn variables{
		.bpm            = mBpm,
		.ppqPosition    = mPpqPosition,
		.loopLength     = mLoopLength,
		.startLength    = mStartLength,
		.endLength      = mEndLength,
		.speed          = mSpeed,
		.mix            = mMix,
		.internalVolume = mInternalVolume,
		.fade           = mFade,
		.phase          = mPhase,
		.inputHead      = mInputHead,
		.outputHead     = mOutputHead
	};

	return variables;
}