# Synthi

A personal playground for exploring **Apple's Audio Unit (AUv3)** framework and deepening my understanding of **Swift / C++ / Objective-C interop**.

## What's here

- **Synthi** — The host app that loads and drives the Audio Unit.
- **SynthiExtension** — An AUv3 instrument extension with DSP oscillators (sine, saw, square, triangle), a parameter model, and a SwiftUI interface.
- DSP code written in **C++** (`SynthiExtension/DSP/`) and called directly from **Swift**.

## Why

This is a learning project to explore:

- The AUv3 plug-in model and `AVAudioUnit` hosting.
- Passing real-time audio parameters between Swift and C++ DSP code.
- MIDI input handling in an AU host context.
- Building custom SwiftUI parameter controls for an audio unit.

No releases or roadmap. Just experimenting.
