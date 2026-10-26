#include "SlowBeatFilter.h"



static float loopLengthBars(int exponent, int split) {
	auto mult = 2.0f / static_cast<float>(split);
	return static_cast<float>(2 << exponent) / 8.0f * mult;
}

static int sampleBorderLength(int exponent, int fadeSamples, int lengthMult) {
	return exponent == 0 ? 0 : (exponent == 1 ? fadeSamples : (2 << exponent) * lengthMult);
}




InterpolatorPool::InterpolatorPool() {
}

void InterpolatorPool::setChannels(int channels) {
	mChannels = channels;
	for (auto &group : mInterpolators) {
		group.resize(channels);
	}
}

int InterpolatorPool::getFreeInterpolatorIdx() {
	for (int i = 0; i < mInterpolators.size(); i++) {
		if (!mOccupied[i]) {
			mOccupied[i] = true;
			return i;
		}
	}

	mInterpolators.push_back(std::vector<juce::Interpolators::CatmullRom>(mChannels));
	mOccupied.push_back(true);
	return mInterpolators.size() - 1;
}

void InterpolatorPool::releaseInterpolatorIdx(int idx) {
	auto &group = mInterpolators[idx];
	for (auto &interpolator : group) {
		interpolator.reset();
	}

	mOccupied[idx] = false;
}

int InterpolatorPool::processChannel(int idx, int channel, double speed, const float *input, float *output, int amount) {
	return mInterpolators[idx][channel].process(speed, input, output, amount);
}




SampleBlockHead::SampleBlockHead(int headPosition, int lengthSamples, int fadeSamples, float speed, int interpolatorIdx) {
	mInputHead = headPosition;
	mRemainingLength = lengthSamples + fadeSamples;
	mFadeLength = fadeSamples;
	mSpeed = speed;
	mInterpolatorIdx = interpolatorIdx;
}

void SampleBlockHead::checkBuffer(int numSamples) {
	if (mBuffer.getNumSamples() != numSamples) {
		mBuffer.setSize(1, numSamples, false, false, true);
	}
}

void SampleBlockHead::writeSamples(InterpolatorPool &pool, juce::AudioBuffer<float> &input, juce::AudioBuffer<float> &output, int numSamples) {
	checkBuffer(numSamples);
	int processedSamples = 0;

	for (int channel = 0; channel < input.getNumChannels(); channel++) {
		processedSamples = pool.processChannel(mInterpolatorIdx, channel, mSpeed, input.getReadPointer(channel, mInputHead), mBuffer.getWritePointer(0), numSamples);
		output.addFromWithRamp(channel, 0, mBuffer.getReadPointer(0), numSamples, 1, 1);
	}

	mInputHead += processedSamples;
	mRemainingLength -= processedSamples;
}

void SampleBlockHead::forceFade() {
	mRemainingLength = mFadeLength;
}




SlowBeatFilter::SlowBeatFilter() {
}

void SlowBeatFilter::prepareToPlay() {
}

void SlowBeatFilter::prepareProcessBlock(int numChannels, int numSamples, SlowBeatParams &params) {
	auto oldSampleRate = mSampleRate;
	auto oldBpm = mBpm;

	mSampleRate = params.sampleRate;
	if (params.hasParams && FLAG_BPM) {
		mBpm = params.bpm;
	}

	if (oldSampleRate != mSampleRate || oldBpm != mBpm || mChannels != numChannels) {
		mChannels = numChannels;
		mBeatLengthSamples = static_cast<int>(mSampleRate / mBpm * 60.0);

		int newSampleSize = (mBeatLengthSamples + 1) * 32;
		mInternalBuffer.setSize(mChannels, newSampleSize, false, true, false);

		mInterpolators.resize(mChannels);
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

	int split = (params.loopLength & 1) ? 2 : 3; // even = triplet
	mLoopLength = loopLengthBars(static_cast<int>(params.loopLength >> 1), split);

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

void SlowBeatFilter::processBlock(juce::AudioBuffer<float> &buffer, SlowBeatParams &params) {
	int numChannels = buffer.getNumChannels();
	prepareProcessBlock(numChannels, buffer.getNumSamples(), params);

	for (int channel = 0; channel < numChannels; channel++) {
		auto *data = buffer.getWritePointer(channel);
		processChannel(channel, data);
	}

	finishProcessBlock();
}




void SlowBeatFilter::finishPlayback() {
	mInternalBuffer.setSize(0, 0);

	mInputHead = 0;
	mOutputHead = 0;
}

const SlowBeatReturn SlowBeatFilter::getVariables() {
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