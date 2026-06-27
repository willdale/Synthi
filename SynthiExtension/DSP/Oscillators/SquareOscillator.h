//
//  SquareOscillator.h
//  SynthiExtension
//
//  Created by Will Dale on 21/06/2026.
//

#pragma once

#include <numbers>
#include <cmath>

class SquareOscillator final {
public:
    SquareOscillator(double sampleRate = 44100.0) {
        mSampleRate = sampleRate;
    }

    void setFrequency(double frequency) {
        mDeltaOmega = frequency / mSampleRate;
    }

    double process() {
        const double sample = std::sin(mOmega * (std::numbers::pi_v<double> * 2.0));
        mOmega += mDeltaOmega;
        if (mOmega >= 1.0) { mOmega -= 1.0; }
        return (sample >= 0.0) ? 1.0 : -1.0;
    }

private:
    double mOmega = { 0.0 };
    double mDeltaOmega = { 0.0 };
    double mSampleRate = { 0.0 };
};
