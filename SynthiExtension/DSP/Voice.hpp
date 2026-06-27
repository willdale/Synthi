//
//  Voice.hpp
//  SynthiExtension
//

#pragma once

#include <variant>
#include <vector>
#include <cmath>

#include "SinOscillator.h"
#include "SawOscillator.h"
#include "SquareOscillator.h"
#include "TriangleOscillator.h"
#include "OscillatorTypes.h"

using OscillatorVariant = std::variant<SinOscillator, SawOscillator, SquareOscillator, TriangleOscillator>;

struct OscillatorSlot {
    OscillatorVariant oscillator;
    double level = 1.0;
    signed short int detune = 0;
};

class Voice {
public:
    Voice(double sampleRate, int slotCount = 0)
        : mSampleRate(sampleRate)
    {
        if (slotCount > 0) setActiveCount(slotCount);
    }

    void setActiveCount(int count) {
        int newCount = std::max(1, count);
        int oldCount = static_cast<int>(mSlots.size());
        mSlots.resize(newCount);
        for (int i = oldCount; i < newCount; ++i) {
            mSlots[i].oscillator.emplace<SinOscillator>(mSampleRate);
            mSlots[i].level = 1.0;
            mSlots[i].detune = 0;
        }
    }

    int activeCount() const {
        return static_cast<int>(mSlots.size());
    }

    void noteOn(double frequency, double velocity) {
        mVelocity = velocity;
        applyFrequencyToAllSlots(frequency);
    }

    void noteOff() {
        mVelocity = 0.0;
    }

    double process() {
        double mixed = 0.0;
        for (auto& slot : mSlots) {
            double sample = std::visit([](auto& osc) -> double {
                return osc.process();
            }, slot.oscillator);
            mixed += sample * slot.level;
        }
        return mixed * mVelocity;
    }

    void setOscillatorType(int slot, OscillatorType type) {
        if (!validSlot(slot)) return;
        auto& variant = mSlots[slot].oscillator;
        if (static_cast<OscillatorType>(variant.index()) == type) return;
        switch (type) {
            case OscillatorType::sine: variant.emplace<SinOscillator>(mSampleRate); break;
            case OscillatorType::saw: variant.emplace<SawOscillator>(mSampleRate); break;
            case OscillatorType::square: variant.emplace<SquareOscillator>(mSampleRate); break;
            case OscillatorType::triangle: variant.emplace<TriangleOscillator>(mSampleRate); break;
        }
        applyFrequencyToSlot(slot);
    }

    OscillatorType getOscillatorType(int slot) const {
        if (!validSlot(slot)) return OscillatorType::sine;
        return static_cast<OscillatorType>(mSlots[slot].oscillator.index());
    }
    
    // MARK: Level
    
    double getOscillatorLevel(int slot) const {
        if (!validSlot(slot)) return 0.0;
        return mSlots[slot].level;
    }

    void setOscillatorLevel(int slot, double level) {
        if (!validSlot(slot)) return;
        mSlots[slot].level = level;
    }
    
    // MARK: Detune
    
    signed short int getOscillatorDetune(int slot) const {
        if (!validSlot(slot)) return 0;
        return mSlots[slot].detune;
    }

    void setOscillatorDetune(int slot, signed short int cent) {
        if (!validSlot(slot)) return;
        mSlots[slot].detune = cent;
    }

private:
    bool validSlot(int slot) const {
        return slot >= 0 && slot < static_cast<int>(mSlots.size());
    }

    void applyFrequencyToAllSlots(double freq) {
        mBaseFrequency = freq;
        for (int i = 0; i < static_cast<int>(mSlots.size()); ++i) {
            applyFrequencyToSlot(i);
        }
    }

    void applyFrequencyToSlot(int slot) {
        double detuned = mBaseFrequency * std::pow(2.0, mSlots[slot].detune / 1200.0);
        std::visit([detuned](auto& osc) {
            osc.setFrequency(detuned);
        }, mSlots[slot].oscillator);
    }

    double mSampleRate;
    double mBaseFrequency = 440.0;
    double mVelocity = 0.0;
    std::vector<OscillatorSlot> mSlots;
};
