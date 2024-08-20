//
//  ContentView.swift
//  audio-playground
//
//  Created by Eric O'Connell on 8/17/24.
//

import SwiftUI

let queue = DispatchSerialQueue(label: "audio")

struct ContentView: View {

    @State
    var width: CGFloat = 0

    @State
    var samples: [Float] = []

    let timer = Timer.publish(every: 0.1, on: .main, in: .common).autoconnect()

    var body: some View {
        VStack {
            Image(systemName: "keyboard")
                .imageScale(.large)
                .foregroundStyle(.tint)
            Text("Synthesizer World!")
            HStack {
                Button(action: {
                    queue.async {
                        synthPlay()
                    }
                }, label: {
                    HStack {
                        Image(systemName: "play")
                        Text("Play")
                    }
                })

                Button(action: {
                    queue.async {
                        synthPause()
                    }
                }, label: {
                    HStack {
                        Image(systemName: "pause")
                        Text("Pause")
                    }
                })

                Button(action: {
                    queue.async {
                        synthStop()
                    }
                }, label: {
                    HStack {
                        Image(systemName: "stop")
                        Text("Stop")
                    }
                })
            }

            Canvas { context, size in
                DispatchQueue.main.async {
                    self.width = size.width
                }

                if (samples.count < Int(width) || samples.count == 0) {
                    return
                }

                let baseline = size.height / 2

                let wave = Path { p in
                    p.move(
                        to: CGPoint(
                            x: 0,
                            y: baseline + CGFloat(samples[0]) * size.height / 2
                        )
                    )
                    for i in 1..<Int(width) {
                        p.addLine(
                            to: CGPoint(
                                x: CGFloat(i),
                                y: baseline + CGFloat(samples[i]) * size.height / 2
                            )
                        )
                    }
                }

                context.stroke(wave, with: .color(.blue))
            }
        }
        .padding()
        .onReceive(timer) { _ in
            var num: Int32 = 0
            let rawSamples = synthGetAudioData(&num)
            if num > 0, width > 0, let rawSamples {
                let sampleCount = Int(width)
                if (samples.count != sampleCount) {
                    print("Making samples count: \(sampleCount)")
                    samples = [Float](repeating: 0, count: sampleCount)
                }

                for i in 0 ..< Int(width) {
                    let s: Float = Float(i) / Float(width) * Float(num - 1)
                    let f1 = s.truncatingRemainder(dividingBy: 1)
                    let f2 = 1 - f1

                    let s1 = f1 * rawSamples[Int(s)]
                    let s2 = f2 * rawSamples[Int(s) + 2]

                    samples[i] = 0.5 * (s1 + s2) // (rawSamples[i * 2] + rawSamples[i * 2 + 1]) // Int(s)]
                }
            }
        }
    }
}

#Preview {
    ContentView()
}
