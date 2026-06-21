//
//  SynthiExtensionAudioUnit.swift
//  SynthiExtension
//
//  Created by Will Dale on 21/06/2026.
//

import AVFoundation

struct KernelWrapper: ~Copyable {
    var value: SynthiExtensionDSPKernel

    init() {
        value = SynthiExtensionDSPKernel()
    }
}

public class SynthiExtensionAudioUnit: AUAudioUnit, @unchecked Sendable
{
	private var kernel = KernelWrapper()
    private var processHelper: AUProcessHelper

	private var outputBus: AUAudioUnitBus
	private var _outputBusses: AUAudioUnitBusArray!

	private var format: AVAudioFormat

	@objc override init(componentDescription: AudioComponentDescription, options: AudioComponentInstantiationOptions) throws {
		format = AVAudioFormat(standardFormatWithSampleRate: 44_100, channels: 2)!
        processHelper = AUProcessHelper(&kernel.value)
        outputBus = try AUAudioUnitBus(format: self.format)
        outputBus.maximumChannelCount = 2
        try super.init(componentDescription: componentDescription, options: options)
        _outputBusses = AUAudioUnitBusArray(audioUnit: self, busType: AUAudioUnitBusType.output, busses: [outputBus])
	}

	public override var outputBusses: AUAudioUnitBusArray {
		return _outputBusses
	}
    
    public override var  maximumFramesToRender: AUAudioFrameCount {
        get {
            return kernel.value.maximumFramesToRender()
        }

        set {
            kernel.value.setMaximumFramesToRender(newValue)
        }
    }

    public override var  shouldBypassEffect: Bool {
        get {
            return kernel.value.isBypassed()
        }

        set {
            kernel.value.setBypass(newValue)
        }
    }

    // MARK: - MIDI
    public override var audioUnitMIDIProtocol: MIDIProtocolID {
        return kernel.value.AudioUnitMIDIProtocol()
    }

    // MARK: - Rendering
    public override var internalRenderBlock: AUInternalRenderBlock {
        return processHelper.internalRenderBlock()
    }
        
    // Allocate resources required to render.
    // Subclassers should call the superclass implementation.
    public override func allocateRenderResources() throws {
		let outputChannelCount = self.outputBusses[0].format.channelCount
        
        kernel.value.setMusicalContextBlock(self.musicalContextBlock)
		kernel.value.initialize(Int32(outputChannelCount), outputBus.format.sampleRate)

        processHelper.setChannelCount(0, self.outputBusses[0].format.channelCount)

		try super.allocateRenderResources()
	}

    // Deallocate resources allocated in allocateRenderResourcesAndReturnError:
    // Subclassers should call the superclass implementation.
    public override func deallocateRenderResources() {
        
        // Deallocate your resources.
        kernel.value.deInitialize()
        
        super.deallocateRenderResources()
    }

	public func setupParameterTree(_ parameterTree: AUParameterTree) {
		self.parameterTree = parameterTree

		// Set the Parameter default values before setting up the parameter callbacks
		for param in parameterTree.allParameters {
            kernel.value.setParameter(param.address, param.value)
		}

		setupParameterCallbacks()
	}

	private func setupParameterCallbacks() {
		// implementorValueObserver is called when a parameter changes value.
		parameterTree?.implementorValueObserver = { [weak self] param, value -> Void in
            self?.kernel.value.setParameter(param.address, value)
		}

		// implementorValueProvider is called when the value needs to be refreshed.
		parameterTree?.implementorValueProvider = { [weak self] param in
            return self?.kernel.value.getParameter(param.address) ?? 0.0
		}

		// A function to provide string representations of parameter values.
		parameterTree?.implementorStringFromValueCallback = { param, valuePtr in
			guard let value = valuePtr?.pointee else {
				return "-"
			}
			return value.formatted(.number)
		}
	}
}
