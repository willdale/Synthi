//
//  SawOscillator.h
//  SynthiExtension
//
//  Created by Will Dale on 21/06/2026.
//

#pragma once

#include <numbers>
#include <cmath>

class SawOscillator {
public:
    SawOscillator(double sampleRate = 44100.0) {
        mSampleRate = sampleRate;
    }

    void setFrequency(double frequency) {
        mDeltaOmega = frequency / mSampleRate;
    }

    double process() {
        // Map phase [0, 1) to [-1, 1)
        const double sample = (mOmega * 2.0) - 1.0;
        mOmega += mDeltaOmega;
        if (mOmega >= 1.0) { mOmega -= 1.0; }
        return sample;
    }

private:
    double mOmega = { 0.0 };
    double mDeltaOmega = { 0.0 };
    double mSampleRate = { 0.0 };
};
