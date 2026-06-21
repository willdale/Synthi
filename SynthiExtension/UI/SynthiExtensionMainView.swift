//
//  SynthiExtensionMainView.swift
//  SynthiExtension
//
//  Created by Will Dale on 21/06/2026.
//

import SwiftUI

struct SynthiExtensionMainView: View {
    var parameterTree: ObservableAUParameterGroup

    /// Strongly‑typed access to the oscillator type parameter.
    private var oscillatorParam: ObservableAUParameter {
        parameterTree.global.oscillatorType as! ObservableAUParameter
    }

    /// A SwiftUI Binding that maps between the parameter’s Float value and our enum.
    private var oscillatorType: Binding<OscillatorType> {
        Binding<OscillatorType>(
            get: {
                let raw = Int32(oscillatorParam.value.rounded())
                return OscillatorType(rawValue: raw) ?? .sine
            },
            set: { newType in
                let newValue = Float(newType.rawValue)
                guard oscillatorParam.value != newValue else { return }
                oscillatorParam.value = newValue
                oscillatorParam.onEditingChanged(true)
                oscillatorParam.onEditingChanged(false)
            }
        )
    }

    var body: some View {
        VStack {
            ParameterSlider(param: parameterTree.global.gain)
            OscillatorTypePicker(oscillatorType: oscillatorType)
        }
    }
}
