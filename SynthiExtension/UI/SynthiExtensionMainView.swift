//
//  SynthiExtensionMainView.swift
//  SynthiExtension
//
//  Created by Will Dale on 21/06/2026.
//

import SwiftUI

struct SynthiExtensionMainView: View {
    var parameterTree: ObservableAUParameterGroup
    
    var body: some View {
        ParameterSlider(param: parameterTree.global.gain)
    }
}
