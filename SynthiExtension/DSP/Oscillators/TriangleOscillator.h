//
//  TriangleOscillator.h
//  SynthiExtension
//
//  Created by Will Dale on 21/06/2026.
//

#pragma once

#include <numbers>
#include <cmath>

class TriangleOscillator {
public:
    TriangleOscillator(double sampleRate = 44100.0) {
        mSampleRate = sampleRate;
    }

    void setFrequency(double frequency) {
        mDeltaOmega = frequency / mSampleRate;
    }

    double process() {
        // Phase [0, 1) -> [-1, 1) triangular wave
        double sample = (mOmega * 4.0) - 2.0;
        if (sample < -1.0) {
            sample = -2.0 - sample;
        } else if (sample > 1.0) {
            sample = 2.0 - sample;
        }
        mOmega += mDeltaOmega;
        if (mOmega >= 1.0) { mOmega -= 1.0; }
        return sample;
    }

private:
    double mOmega = { 0.0 };
    double mDeltaOmega = { 0.0 };
    double mSampleRate = { 0.0 };
};
