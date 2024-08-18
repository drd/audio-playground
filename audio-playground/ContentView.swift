//
//  ContentView.swift
//  audio-playground
//
//  Created by Eric O'Connell on 8/17/24.
//

import SwiftUI

struct ContentView: View {
    var body: some View {
        VStack {
            Image(systemName: "globe")
                .imageScale(.large)
                .foregroundStyle(.tint)
            Text("Hello, world!")
            Button(action: {
                play()
            }, label: {
                HStack {
                    Image(systemName: "play")
                    Text("Play")
                }
            })

            Button(action: {
                stop()
            }, label: {
                HStack {
                    Image(systemName: "stop")
                    Text("Stop")
                }
            })
        }
        .padding()
    }
}

#Preview {
    ContentView()
}
