//
//  OscillatorTypePicker.swift
//  SynthiExtension
//
//

import SwiftUI

struct OscillatorTypePicker: View {
    @Binding var oscillatorType: OscillatorType

    var body: some View {
        Picker("Oscillator Type", selection: $oscillatorType) {
            ForEach(OscillatorType.allCases, id: \.self) { type in
                Text(type.displayName).tag(type)
            }
        }
        .pickerStyle(.segmented)
        .padding()
    }
}

#Preview {
    @Previewable @State var type: OscillatorType = .sine
    return OscillatorTypePicker(oscillatorType: $type)
}
