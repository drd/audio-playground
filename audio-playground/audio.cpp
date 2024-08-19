//
//  audio.c
//  audio-playground
//
//  Created by Eric O'Connell on 8/17/24.
//

#include "audio.h"

std::vector<Pattern> patterns = {
    { .notes = {
        { Note::A, 3, 0.7f },
        { Note::A, 3, 0.0f },
//        { Note::B, 3, 0.3f, 0.5f },
//        { Note::C, 3, 0.5f, 0.5f },
//        { Note::A, 3, 0.7f },
    } },
    { .notes = {
        { Note::G, 3, 0.7f },
        { Note::F, 3, 0.5f },
        { Note::E, 3, 0.6f },
        { Note::E, 3, 0.0f },
        { Note::F, 3, 0.5f },
        { Note::E, 3, 0.3f },
        { Note::E, 3, 0.0f },
    } },
    { .notes = {
        { Note::E, 3, 0.2f },
        { Note::F, 3, 0.5f },
        { Note::E, 3, 0.0f },
        { Note::A, 3, 0.7f },
        { Note::G, 3, 0.9f },
        { Note::C, 3, 0.6f },
        { Note::E, 3, 0.0f },
        { Note::C, 3, 0.7f },
        { Note::A, 3, 0.7f },
    } },
    { .notes = {
        { Note::E, 4, 0.3f },
        { Note::E, 3, 0.0f },
        { Note::E, 4, 0.8f },
        { Note::E, 3, 0.0f },
        { Note::E, 3, 0.0f },
        { Note::F, 4, 0.5f },
        { Note::A, 5, 0.7f },
        { Note::D, 5, 0.7f },
    } },
};

std::vector<Channel> channels = {
    {
        .patternIndex = {0, 1, 2},
        .oscillator = Oscillator::Sine,
        .timeFactor = 1.0f,
    },
    {
        .patternIndex = {2, 0, 1},
        .oscillator = Oscillator::Square,
        .timeFactor = 1.0f
    },
    {
        .patternIndex = {3, 2},
        .oscillator = Oscillator::Noise,
        .timeFactor = 1.0f,
    }
};

Song song = {
    .tempo = 10.0f,
    .patterns = patterns,
    .channels = channels
};

Engine engine = { song };

void synthPlay() {
    engine.play();
}

void synthPause() {
    engine.pause();
}

void synthStop() {
    engine.stop();
}

float oscBuffer[BUFFER_SIZE];

float* synthGetAudioData(int* numSamples) {
    if (!engine.lastBuffer) {
        *numSamples = 0;
        return nullptr;
    }

    *numSamples = BUFFER_SIZE;
    for (auto i = 0; i < BUFFER_SIZE; i++) {
        oscBuffer[i] = engine.lastBuffer[i];
    }
    return oscBuffer;
}

void callback(void *custom_data, AudioQueueRef queue, AudioQueueBufferRef buffer) {
    auto engine = static_cast<Engine*>(custom_data);
    engine->fillBuffer(queue, buffer);
}
