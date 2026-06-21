//
//  SynthiApp.swift
//  Synthi
//
//  Created by Will Dale on 21/06/2026.
//

import SwiftUI

@main
struct SynthiApp: App {
    private let hostModel = AudioUnitHostModel()

    var body: some Scene {
        WindowGroup {
            ContentView(hostModel: hostModel)
        }
    }
}
