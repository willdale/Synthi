//
//  Parameters.swift
//  SynthiExtension
//
//  Created by Will Dale on 21/06/2026.
//

import Foundation
import AudioToolbox

let SynthiExtensionParameterSpecs = ParameterTreeSpec {
    ParameterGroupSpec(identifier: "global", name: "Global") {
        ParameterSpec(
            address: .gain,
            identifier: "gain",
            name: "Output Gain",
            units: .linearGain,
            valueRange: 0.0...1.0,
            defaultValue: 0.25
        )
        ParameterSpec(
            address: .oscillatorType,
            identifier: "oscillatorType",
            name: "Oscillator Type",
            units: .indexed,
            valueRange: 0.0...3.0,
            defaultValue: 0.0,
            valueStrings: ["Sine", "Saw", "Square", "Triangle"]
        )
    }
}

extension ParameterSpec {
    init(
        address: SynthiExtensionParameterAddress,
        identifier: String,
        name: String,
        units: AudioUnitParameterUnit,
        valueRange: ClosedRange<AUValue>,
        defaultValue: AUValue,
        unitName: String? = nil,
        flags: AudioUnitParameterOptions = [AudioUnitParameterOptions.flag_IsWritable, AudioUnitParameterOptions.flag_IsReadable],
        valueStrings: [String]? = nil,
        dependentParameters: [NSNumber]? = nil
    ) {
        self.init(address: address.rawValue,
                  identifier: identifier,
                  name: name,
                  units: units,
                  valueRange: valueRange,
                  defaultValue: defaultValue,
                  unitName: unitName,
                  flags: flags,
                  valueStrings: valueStrings,
                  dependentParameters: dependentParameters)
    }
}

// MARK: - Oscillator type mapping

/// Represents the available oscillator waveforms (matches C++ variant indices).
enum OscillatorType: Int, CaseIterable, Sendable {
    case sine   = 0
    case saw    = 1
    case square = 2
    case triangle = 3

    var displayName: String {
        switch self {
        case .sine:      return "Sine"
        case .saw:       return "Saw"
        case .square:    return "Square"
        case .triangle:  return "Triangle"
        }
    }
}
