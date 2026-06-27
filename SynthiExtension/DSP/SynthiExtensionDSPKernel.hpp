//
//  SynthiExtensionDSPKernel.hpp
//  SynthiExtension
//
//  Created by Will Dale on 21/06/2026.
//

#pragma once

#import <swift/bridging>

#import <AudioToolbox/AudioToolbox.h>
#import <CoreMIDI/CoreMIDI.h>
#import <algorithm>
#import <span>

#import "Voice.hpp"
#import "SynthiExtensionParameterAddresses.h"
#import "OscillatorTypes.h"

class SWIFT_SELF_CONTAINED SWIFT_NONCOPYABLE SWIFT_ESCAPABLE SynthiExtensionDSPKernel final {
public:
    SynthiExtensionDSPKernel() = default;
    SynthiExtensionDSPKernel(const SynthiExtensionDSPKernel&) = delete;
    SynthiExtensionDSPKernel& operator=(const SynthiExtensionDSPKernel&) = delete;
    SynthiExtensionDSPKernel(SynthiExtensionDSPKernel&&) = default;
    SynthiExtensionDSPKernel& operator=(SynthiExtensionDSPKernel&&) = default;
    
    void initialize(int channelCount, double inSampleRate) {
        mSampleRate = inSampleRate;
        mVoice = Voice(inSampleRate, mOscillatorCount);
    }
    
    void deInitialize() {
        mMusicalContextBlock = nullptr;
    }
    
    // MARK: - Bypass
    
    bool isBypassed() const {
        return mBypassed;
    }
    
    void setBypass(bool shouldBypass) {
        mBypassed = shouldBypass;
    }
    
    // MARK: - Parameter Getter / Setter
    
    void setParameter(AUParameterAddress address, AUValue value) {
        if (address == SynthiExtensionParameterAddress::gain) {
            mGain = value;
        } else if (address == SynthiExtensionParameterAddress::oscillatorCount) {
            mOscillatorCount = static_cast<int>(value);
            mVoice.setActiveCount(mOscillatorCount);
        } else {
            OscAddress osc;
            if (decodeOscAddress(address, osc)) {
                switch (osc.param) {
                    case 0: mVoice.setOscillatorType(osc.slot, static_cast<OscillatorType>(static_cast<int>(value))); break;
                    case 1: mVoice.setOscillatorLevel(osc.slot, value); break;
                    case 2: mVoice.setOscillatorDetune(osc.slot, static_cast<signed short int>(value)); break;
                }
            }
        }
    }

    AUValue getParameter(AUParameterAddress address) {
        if (address == SynthiExtensionParameterAddress::gain) {
            return (AUValue)mGain;
        }
        if (address == SynthiExtensionParameterAddress::oscillatorCount) {
            return (AUValue)mOscillatorCount;
        }
        OscAddress osc;
        if (decodeOscAddress(address, osc)) {
            if (osc.slot >= mVoice.activeCount()) return 0.f;
            switch (osc.param) {
                case 0: return static_cast<AUValue>(mVoice.getOscillatorType(osc.slot));
                case 1: return static_cast<AUValue>(mVoice.getOscillatorLevel(osc.slot));
                case 2: return static_cast<AUValue>(mVoice.getOscillatorDetune(osc.slot));
            }
        }
        return 0.f;
    }
    
    // MARK: - Max Frames
    
    AUAudioFrameCount maximumFramesToRender() const {
        return mMaxFramesToRender;
    }
    
    void setMaximumFramesToRender(const AUAudioFrameCount &maxFrames) {
        mMaxFramesToRender = maxFrames;
    }
    
    // MARK: - Musical Context
    
    void setMusicalContextBlock(AUHostMusicalContextBlock contextBlock) {
        mMusicalContextBlock = contextBlock;
    }
    
    // MARK: - MIDI Protocol
    
    MIDIProtocolID AudioUnitMIDIProtocol() const {
        return kMIDIProtocol_2_0;
    }
    
    inline double MIDINoteToFrequency(int note) {
        constexpr auto kMiddleA = 440.0;
        return (kMiddleA / 32.0) * pow(2, ((note - 9) / 12.0));
    }
    
    void process(std::span<float *> outputBuffers, AUEventSampleTime bufferStartTime, AUAudioFrameCount frameCount) {
        if (mBypassed) {
            for (UInt32 channel = 0; channel < outputBuffers.size(); ++channel) {
                std::fill_n(outputBuffers[channel], frameCount, 0.f);
            }
            return;
        }
        
        if (mMusicalContextBlock) {
            mMusicalContextBlock(&currentTempo,
                                 &timeSignatureNumerator,
                                 &timeSignatureDenominator,
                                 &currentBeatPosition,
                                 &sampleOffsetToNextBeat,
                                 &currentMeasureDownbeatPosition);
        }
        
        for (UInt32 frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
            double sample = mVoice.process() * mGain;
            for (UInt32 channel = 0; channel < outputBuffers.size(); ++channel) {
                outputBuffers[channel][frameIndex] = sample;
            }
        }
    }
    
    void handleOneEvent(AUEventSampleTime now, AURenderEvent const *event) {
        switch (event->head.eventType) {
            case AURenderEventParameter: {
                handleParameterEvent(now, event->parameter);
                break;
            }
            case AURenderEventMIDIEventList: {
                handleMIDIEventList(now, &event->MIDIEventsList);
                break;
            }
            default:
                break;
        }
    }
    
    void handleParameterEvent(AUEventSampleTime now, AUParameterEvent const& parameterEvent) {
        setParameter(parameterEvent.parameterAddress, parameterEvent.value);
    }
    
    void handleMIDIEventList(AUEventSampleTime now, AUMIDIEventList const* midiEvent) {
        auto visitor = [] (void* context, MIDITimeStamp timeStamp, MIDIUniversalMessage message) {
            auto thisObject = static_cast<SynthiExtensionDSPKernel *>(context);
            switch (message.type) {
                case kMIDIMessageTypeChannelVoice2: {
                    thisObject->handleMIDI2VoiceMessage(message);
                }
                    break;
                default:
                    break;
            }
        };
        MIDIEventListForEachEvent(&midiEvent->eventList, visitor, this);
    }
    
    void handleMIDI2VoiceMessage(const struct MIDIUniversalMessage& message) {
        const auto& note = message.channelVoice2.note;
        switch (message.channelVoice2.status) {
            case kMIDICVStatusNoteOff: {
                mVoice.noteOff();
            }
                break;
            case kMIDICVStatusNoteOn: {
                const auto velocity = message.channelVoice2.note.velocity;
                double freq = MIDINoteToFrequency(note.number);
                double normVelocity = (double)velocity / (double)std::numeric_limits<std::uint16_t>::max();
                mVoice.noteOn(freq, normVelocity);
            }
                break;
            default:
                break;
        }
    }
    
    // MARK: - Member Variables
    
    AUHostMusicalContextBlock mMusicalContextBlock = nullptr;
    
    double mSampleRate = 44100.0;
    double mGain = 1.0;
    
    bool mBypassed = false;
    AUAudioFrameCount mMaxFramesToRender = 1024;
    
    int mOscillatorCount = 3;
    Voice mVoice{44100.0, mOscillatorCount};
    
    double currentTempo = 120.0;
    double timeSignatureNumerator = 4.0;
    long timeSignatureDenominator = 4;
    double currentBeatPosition = 0.0;
    long sampleOffsetToNextBeat = 0;
    double currentMeasureDownbeatPosition = 0.0;
};
