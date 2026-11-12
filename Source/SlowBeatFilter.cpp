#include "SlowBeatFilter.h"








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

int InterpolatorPool::processChannel(int idx, int channel, double speed, const float *input, float *output, int amount, int maxInput) {
	return mInterpolators[idx][channel].process(speed, input, output, amount, maxInput, 0);
}








SampleBlockHead::SampleBlockHead(int headPosition, int lengthSamples, int fadeSamples, float speed, InterpolatorPool &pool)
	: mAssignedPool(pool), mInputHead(headPosition), mFadeLength(fadeSamples), mSpeed(speed) {
	mRemainingLength = lengthSamples + fadeSamples;
	mIdx = pool.getFreeInterpolatorIdx();
}

SampleBlockHead::~SampleBlockHead() {
	mAssignedPool.releaseInterpolatorIdx(mIdx);
}




void SampleBlockHead::checkBuffer(int numSamples) {
	if (mBuffer.getNumSamples() != numSamples) {
		mBuffer.setSize(1, numSamples, false, false, true);
	}
}

int SampleBlockHead::sampleMin(int a, int b) {
	return a > b ? b : a;
}

int SampleBlockHead::writeSamplesRaw(
	const juce::AudioBuffer<float> &input, juce::AudioBuffer<float> &output,
	int inputHead, int outputHead, int numSamples, float startGain, float endGain
) {
	int processedSamples = 0;
	for (int channel = 0; channel < input.getNumChannels(); channel++) {

		processedSamples = mAssignedPool.processChannel(
			mIdx, channel, mSpeed,
			input.getReadPointer(channel, inputHead),
			mBuffer.getWritePointer(outputHead),
			numSamples
		);

		output.addFromWithRamp(
			channel, outputHead,
			mBuffer.getReadPointer(outputHead),
			numSamples, startGain, endGain
		);
	}

	return processedSamples;
}

int SampleBlockHead::writeSamples(
	const juce::AudioBuffer<float> &input, juce::AudioBuffer<float> &output,
	int inputHead, int outputHead, int numSamples, float startGain, float endGain
) {
	return 0; // todo: check if mInputHead and subsequent read samples end after input buffer border
	// if it does, stop early, and play more from the beginning of the buffer (i.e. reset mInputHead back to 0)
	// already overloaded mAssignedPool processChannel w/ max input samples
}




void SampleBlockHead::processBlock(const juce::AudioBuffer<float> &input, juce::AudioBuffer<float> &output, int numSamples) {
	if (mRemainingLength <= 0) return;
	checkBuffer(numSamples);

	int processedOutput = 0;

	auto nonFadeOutLength = mRemainingLength - mFadeLength;
	if (nonFadeOutLength > 0) {
		auto nonFadeOutSamples = sampleMin(numSamples, nonFadeOutLength);
		numSamples -= nonFadeOutSamples;

		// fade in stage
		if (mWeight < 1.0f) {
			auto fadeInSamples = sampleMin(
				static_cast<int>((1.0f - mWeight) * static_cast<float>(mFadeLength)),
				nonFadeOutSamples
			);
			auto newWeight = fmax(
				mWeight + static_cast<float>(fadeInSamples) / static_cast<float>(mFadeLength),
				1.0f
			);

			mInputHead += writeSamplesRaw(input, output, mInputHead, 0, fadeInSamples, mWeight, newWeight);
			mWeight = newWeight;

			mRemainingLength -= fadeInSamples;
			nonFadeOutSamples -= fadeInSamples;
			processedOutput += fadeInSamples;
		}

		// full volume stage
		if (nonFadeOutSamples > 0) {
			mInputHead += writeSamplesRaw(input, output, mInputHead, processedOutput, nonFadeOutSamples, 1.0f, 1.0f);
			
			mRemainingLength -= nonFadeOutSamples;
			processedOutput += nonFadeOutSamples;
		}
	}

	// numSamples is changed
	// fade out stage
	if (numSamples > 0) {
		auto fadeOutSamples = sampleMin(
			static_cast<int>((1.0f - mWeight) * static_cast<float>(mFadeLength)),
			sampleMin(mRemainingLength, numSamples)
		);
		auto newWeight = fmin(
			mWeight - static_cast<float>(fadeOutSamples) / static_cast<float>(mFadeLength),
			0.0f
		);

		mInputHead += writeSamplesRaw(input, output, mInputHead, processedOutput, fadeOutSamples, mWeight, newWeight);
		mRemainingLength -= fadeOutSamples;
	}
}

void SampleBlockHead::forceFade() {
	mRemainingLength = mFadeLength;
}








SlowBeatFilter::SlowBeatFilter() {
}

float SlowBeatFilter::loopLengthBars(int exponent, int split) {
	auto mult = 2.0f / static_cast<float>(split);
	return static_cast<float>(2 << exponent) / 8.0f * mult;
}

int SlowBeatFilter::sampleBorderLength(int exponent, int fadeSamples, int lengthMult) {
	return exponent == 0 ? 0 : (exponent == 1 ? fadeSamples : (2 << exponent) * lengthMult);
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

		if (prevModFull > fmod(mPpqPosition, 32.0f) || mInputHead > mInternalBuffer.getNumSamples()) {
			mInputHead = 0;
		}

		if (prevModLoop > fmod(mPpqPosition, mLoopLength) || mOutputHead > mInputHead) {
			mOutputHead = mInputHead;
		}
	}
}




void SlowBeatFilter::processChannel(int channel, float *channelData) {
	float *internalChannelData = mInternalBuffer.getWritePointer(channel, mInputHead);
	for (int i = 0; i < mNumSamples; ++i) {
		internalChannelData[i] = channelData[i];
	}
	mProcessed = mInterpolators[channel].process(mSpeed, mInternalBuffer.getWritePointer(channel, mOutputHead), channelData, mNumSamples);
}

void SlowBeatFilter::processBlock(juce::AudioBuffer<float> &buffer, SlowBeatParams &params) {
	int numChannels = buffer.getNumChannels();
	prepareProcessBlock(numChannels, buffer.getNumSamples(), params);

	if (params.isPlaying && params.on) {
		for (int channel = 0; channel < numChannels; channel++) {
			auto *data = buffer.getWritePointer(channel);
			processChannel(channel, data);
		}
	}

	finishProcessBlock();
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