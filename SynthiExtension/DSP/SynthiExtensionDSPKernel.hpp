//
//  SynthiExtensionDSPKernel.hpp
//  SynthiExtension
//
//  Created by Will Dale on 21/06/2026.
//

#pragma once

#import <AudioToolbox/AudioToolbox.h>
#import <CoreMIDI/CoreMIDI.h>
#import <algorithm>
#import <vector>
#import <span>
#import <variant>

#import "SinOscillator.h"
#import "SawOscillator.h"
#import "SquareOscillator.h"
#import "TriangleOscillator.h"
#import "SynthiExtensionParameterAddresses.h"


class SynthiExtensionDSPKernel final {
public:
    SynthiExtensionDSPKernel() = default;
    SynthiExtensionDSPKernel(const SynthiExtensionDSPKernel&) = delete;
    SynthiExtensionDSPKernel& operator=(const SynthiExtensionDSPKernel&) = delete;
    SynthiExtensionDSPKernel(SynthiExtensionDSPKernel&&) = default;
    SynthiExtensionDSPKernel& operator=(SynthiExtensionDSPKernel&&) = default;

    using OscillatorVariant = std::variant<SinOscillator, SawOscillator, SquareOscillator, TriangleOscillator>;

    void initialize(int channelCount, double inSampleRate) {
        mSampleRate = inSampleRate;
        mOscillator.emplace<SinOscillator>(inSampleRate);
        mOscType = 0;
    }
    
    void deInitialize() {
        mMusicalContextBlock = nullptr;
    }
    
    // MARK: - Bypass
    bool isBypassed() {
        return mBypassed;
    }
    
    void setBypass(bool shouldBypass) {
        mBypassed = shouldBypass;
    }
    
    // MARK: - Parameter Getter / Setter
    void setParameter(AUParameterAddress address, AUValue value) {
        switch (address) {
            case SynthiExtensionParameterAddress::gain:
                mGain = value;
                break;
            case SynthiExtensionParameterAddress::oscillatorType: {
                int newType = static_cast<int>(value);
                if (newType != mOscType) {
                    mOscType = newType;
                    switch (mOscType) {
                        case 0:
                            mOscillator.emplace<SinOscillator>(mSampleRate);
                            break;
                        case 1:
                            mOscillator.emplace<SawOscillator>(mSampleRate);
                            break;
                        case 2:
                            mOscillator.emplace<SquareOscillator>(mSampleRate);
                            break;
                        case 3:
                            mOscillator.emplace<TriangleOscillator>(mSampleRate);
                            break;
                        default:
                            mOscillator.emplace<SinOscillator>(mSampleRate);
                            break;
                    }
                    // Reapply the last known frequency, if any.
                    std::visit([this](auto& osc) {
                        osc.setFrequency(mCurrentFreq);
                    }, mOscillator);
                }
                break;
            }
        }
    }
    
    AUValue getParameter(AUParameterAddress address) {
        switch (address) {
            case SynthiExtensionParameterAddress::gain:
                return (AUValue)mGain;
            case SynthiExtensionParameterAddress::oscillatorType:
                return (AUValue)mOscType;
            default: return 0.f;
        }
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
        // Copy the block to ensure it survives beyond this call.
        mMusicalContextBlock = [contextBlock copy];
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
            double sample = std::visit([](auto& osc) -> double {
                return osc.process();
            }, mOscillator);
            sample *= mNoteEnvelope * mGain;
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
                mNoteEnvelope = 0.0;
            }
                break;
            case kMIDICVStatusNoteOn: {
                const auto velocity = message.channelVoice2.note.velocity;
                mCurrentFreq = MIDINoteToFrequency(note.number);
                std::visit([this](auto& osc) {
                    osc.setFrequency(mCurrentFreq);
                }, mOscillator);
                mNoteEnvelope = (double)velocity / (double)std::numeric_limits<std::uint16_t>::max();
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
    double mNoteEnvelope = 0.0;
    
    bool mBypassed = false;
    AUAudioFrameCount mMaxFramesToRender = 1024;
    
    OscillatorVariant mOscillator;
    int mOscType = 0;
    double mCurrentFreq = 440.0;
    
    double currentTempo = 120.0;
    double timeSignatureNumerator = 4.0;
    long timeSignatureDenominator = 4;
    double currentBeatPosition = 0.0;
    long sampleOffsetToNextBeat = 0;
    double currentMeasureDownbeatPosition = 0.0;
};
