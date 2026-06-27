//
//  SynthiExtensionMainView.swift
//  SynthiExtension
//
//  Created by Will Dale on 21/06/2026.
//

import SwiftUI

struct SynthiExtensionMainView: View {
    var parameterTree: ObservableAUParameterGroup

    private var oscillatorCount: Int {
        let param = parameterTree.global.oscillatorCount as! ObservableAUParameter
        return Int(param.value.rounded())
    }

    private func oscillatorBinding(for param: ObservableAUParameterNode) -> Binding<OscillatorType> {
        let oscParam = param as! ObservableAUParameter
        return Binding<OscillatorType>(
            get: {
                let raw = Int32(oscParam.value.rounded())
                return OscillatorType(rawValue: raw) ?? .sine
            },
            set: { newType in
                let newValue = Float(newType.rawValue)
                guard oscParam.value != newValue else { return }
                oscParam.value = newValue
                oscParam.onEditingChanged(true)
                oscParam.onEditingChanged(false)
            }
        )
    }

    var body: some View {
        ScrollView {
            VStack(spacing: 16) {
                GroupBox("Output") {
                    ParameterSlider(param: parameterTree.global.gain)
                    ParameterStepper(param: parameterTree.global.oscillatorCount)
                }

                ForEach(1...oscillatorCount, id: \.self) { i in
                    if let group = parameterTree.children["osc\(i)"] as? ObservableAUParameterGroup {
                        GroupBox("Oscillator \(i)") {
                            OscillatorTypePicker(oscillatorType: oscillatorBinding(for: group.children["osc\(i)Type"]!))
                            if let levelParam = group.children["osc\(i)Level"] as? ObservableAUParameter {
                                ParameterSlider(param: levelParam)
                            } else {
                                Text("ERROR: Could not get Level parameter")
                            }
                            if let detuneParam = group.children["osc\(i)Detune"] as? ObservableAUParameter {
                                ParameterSlider(param: detuneParam)
                            } else {
                                Text("ERROR: Could not get Detune parameter")
                            }
                        }
                    }
                }
            }
            .padding()
        }
    }
}
