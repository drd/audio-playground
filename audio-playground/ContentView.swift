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

            ScrollView(.horizontal) {
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
                }.frame(width: 4096)
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
                    let s = CGFloat(i) / width * CGFloat(num)
                    samples[i] = rawSamples[i * 2] // Int(s)]
                }
            }
        }
    }
}

#Preview {
    ContentView()
}
