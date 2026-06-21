//
//  OscillatorType.swift
//  Synthi
//
//  Created by Will Dale on 21/06/2026.
//

extension OscillatorType: CaseIterable {
    public static var allCases: [OscillatorType] {
        [.sine, .saw, .square, .triangle]
    }
}

extension OscillatorType {
    var displayName: String {
        switch self {
        case .sine: "Sine"
        case .saw: "Saw"
        case .square: "Square"
        case .triangle: "Triangle"
        @unknown default: "none"
        }
    }
}

extension OscillatorType: @unchecked Sendable {}

extension OscillatorType {
    static var valueRange: ClosedRange<AUValue> {
        let rawValues = Self.allCases.map { AUValue($0.rawValue) }
        guard let minValue = rawValues.min(),
              let maxValue = rawValues.max() else {
            fatalError("OscillatorType must have at least one case.")
        }
        return minValue...maxValue
    }
    
    static var displayNames: [String] {
        OscillatorType.allCases.map(\.displayName)
    }
    
}
