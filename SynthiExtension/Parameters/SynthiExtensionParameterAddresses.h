//
//  SynthiExtensionParameterAddresses.h
//  SynthiExtension
//
//  Created by Will Dale on 21/06/2026.
//

#pragma once

#include <AudioToolbox/AUParameters.h>

static constexpr int kMaxOscillators = 4;
static constexpr int kParamsPerOscillator = 3;

typedef NS_ENUM(AUParameterAddress, SynthiExtensionParameterAddress) {
    gain = 0,
    oscillatorCount = 1,
    // Oscillator parameter addresses use oscTypeAddress / oscLevelAddress / oscLevelDetune
};

inline constexpr AUParameterAddress oscTypeAddress(int slot)   { return 2 + (slot - 1) * 3; }

struct OscAddress { int slot; int param; }; // 0=Type, 1=Level, 2=Detune

inline bool decodeOscAddress(AUParameterAddress address, OscAddress &osc) {
    if (address < oscTypeAddress(1)) return false;
    int offset = static_cast<int>(address - oscTypeAddress(1));
    osc.slot  = offset / kParamsPerOscillator;
    osc.param = offset % kParamsPerOscillator;
    return true;
}
