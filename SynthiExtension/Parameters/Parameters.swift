//
//  Parameters.swift
//  SynthiExtension
//
//  Created by Will Dale on 21/06/2026.
//

import Foundation
import AudioToolbox

func oscAddress(_ slot: Int, _ param: Int) -> AUParameterAddress {
    return AUParameterAddress(2 + (slot - 1) * 3 + param)
}

let SynthiExtensionParameterSpecs: ParameterTreeSpec = {
    var children: [NodeSpec] = []

    let global = ParameterGroupSpec(identifier: "global", name: "Global") {
        ParameterSpec(
            address: .gain,
            identifier: "gain",
            name: "Output Gain",
            units: .linearGain,
            valueRange: 0.0...1.0,
            defaultValue: 0.25
        )
        ParameterSpec(
            address: .oscillatorCount,
            identifier: "oscillatorCount",
            name: "Oscillators",
            units: .indexed,
            valueRange: 1...AUValue(kMaxOscillators),
            defaultValue: 3
        )
    }
    children.append(global)

    for i in 1...Int(kMaxOscillators) {
        let group = ParameterGroupSpec(identifier: "osc\(i)", name: "Oscillator \(i)") {
            ParameterSpec(
                address: oscAddress(i, 0),
                identifier: "osc\(i)Type",
                name: "Type",
                units: .indexed,
                valueRange: OscillatorType.valueRange,
                defaultValue: AUValue(OscillatorType.sine.rawValue),
                valueStrings: OscillatorType.displayNames
            )
            ParameterSpec(
                address: oscAddress(i, 1),
                identifier: "osc\(i)Level",
                name: "Level",
                units: .linearGain,
                valueRange: 0.0...1.0,
                defaultValue: i == 1 ? 1.0 : (i == 2 ? 0.5 : 0.3)
            )
            ParameterSpec(
                address: oscAddress(i, 2),
                identifier: "osc\(i)Detune",
                name: "Detune",
                units: .cents,
                valueRange: -100.0...100.0,
                defaultValue: 0.0
            )
        }
        children.append(group)
    }

    return ParameterTreeSpec(children: children)
}()

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
        self.init(
            address: address.rawValue,
            identifier: identifier,
            name: name,
            units: units,
            valueRange: valueRange,
            defaultValue: defaultValue,
            unitName: unitName,
            flags: flags,
            valueStrings: valueStrings,
            dependentParameters: dependentParameters
        )
    }
}
