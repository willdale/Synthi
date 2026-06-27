//
//  ParameterStepper.swift
//  SynthiExtension
//

import SwiftUI

struct ParameterStepper: View {
    @State var param: ObservableAUParameter

    private var intValue: Binding<Int> {
        Binding<Int>(
            get: { Int(param.value.rounded()) },
            set: { newValue in
                let clamped = newValue.clamped(to: Int(param.min.rounded())...Int(param.max.rounded()))
                let newFloat = Float(clamped)
                guard param.value != newFloat else { return }
                param.value = newFloat
                param.onEditingChanged(true)
                param.onEditingChanged(false)
            }
        )
    }

    var body: some View {
        Stepper(
            value: intValue,
            in: Int(param.min.rounded())...Int(param.max.rounded())
        ) {
            Text("\(param.displayName): \(intValue.wrappedValue)")
        }
        .padding(.horizontal)
    }
}

extension Int {
    func clamped(to range: ClosedRange<Int>) -> Int {
        Swift.min(Swift.max(self, range.lowerBound), range.upperBound)
    }
}
