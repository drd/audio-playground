//
//  audio.h
//  audio-playground
//
//  Created by Eric O'Connell on 8/17/24.
//

#pragma once

#include <cstdlib>
#include <cmath>

#include <vector>

#include <AudioToolbox/AudioQueue.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreFoundation/CFRunLoop.h>


#define NUM_BUFFERS 3
#define BUFFER_SIZE 4096
#define SAMPLE_TYPE float
#define MAX_NUMBER 1.0
#define SAMPLE_RATE 44100

extern "C" {
void play();
void stop();
}

struct Envelope {
    float a;
    float d;
    float s;
    float r;
};

enum class Note {
    A = 0,
    A$,
    B,
    C,
    C$,
    D,
    D$,
    E,
    F,
    F$,
    G,
    G$,
};

float frequency(Note note, uint32_t octave) {
    float octaveSemitones = (float(octave) - 4) * 12;
    float noteSemitones = static_cast<int>(note) - static_cast<int>(Note::A);

    float f = pow(2.0f, (octaveSemitones + noteSemitones) / 12.0f) * 440.0f;
//    printf("Note: %d octave: %u semitones: %.1f freq: %.3f", static_cast<int>(note), octave, octaveSemitones + noteSemitones, f);
    return f;
}


struct NoteOn {
    Note note;
    uint32_t octave;
//    float length;
    float intensity;
};

enum class Oscillator {
    Square,
    Triangle,
    Sine,
    Noise
};

struct Pattern {
    std::vector<NoteOn> notes;
};

struct Channel {
    std::vector<size_t> patternIndex;
    Oscillator oscillator;
};

struct Song {
    std::vector<Pattern> patterns;
    std::vector<Channel> channels;
};

void callback(void *custom_data, AudioQueueRef queue, AudioQueueBufferRef buffer);

struct ChannelState {
    Oscillator oscillator;
    size_t channel = 0;
    size_t currentPattern = 0; // is an index into the index :)
    size_t currentNote = 0;

    float frequency = 0;
    float intensity = 0;

    float patternStart = 0.0f;
    float noteStart = 0.0f;
    bool nextPattern = false;
    bool nextNote = false;
//    bool nextTick = false;
};

struct Engine {
    size_t timeIndex = 0;
    float time;
    float beat;
//    float note;
//    float tick;

    std::vector<ChannelState> channels;
    const Song& song;
    AudioQueueRef queue;

    Engine(const Song& song)
    : song(song)
    {}

    void play() {
        channels.clear();
        size_t i = 0;
        for (const auto& channel : song.channels) {
            channels.push_back({
                .oscillator = channel.oscillator,
                .channel = i++,
                .currentPattern = 0,
                .currentNote = 0
            });
        }
        initializeAudio();
    }

    void stop() {
        AudioQueueStop(queue, true);
        CFRunLoopStop(CFRunLoopGetCurrent());
        timeIndex = 0;
    }

    void tick() {
        timeIndex += 1;
        time = float(timeIndex) / SAMPLE_RATE;

        beat = time * 120.0 / 60.0;

        for (auto& state: channels) {
//        auto&state = channels[1];
            auto c = song.channels[state.channel];
            auto p = song.patterns[c.patternIndex[state.currentPattern]];

            if (beat - state.patternStart > p.notes.size()) {
                // next pattern!
                const auto& patternTable = song.channels[state.channel].patternIndex;
                state.currentPattern = (state.currentPattern + 1) % patternTable.size();
                state.nextNote = state.nextPattern = true;
                state.noteStart = state.patternStart = floor(beat);

                p = song.patterns[state.currentPattern];
//                printf("t[%.3f] b[%.3f] New pattern: %zu\n", time, beat, state.currentPattern);
            }

            auto note = floor(beat - state.patternStart);
            if (note != state.currentNote) {
                state.noteStart = floor(beat);
//                printf("t[%.3f] b[%.3f] New note: %.1f\n", time, beat, note);
            }

            const auto& n = p.notes[note];
//            state.oscillator = n.oscillator;
            state.frequency = frequency(n.note, n.octave);
            state.intensity = n.intensity;
        }
    }

    void fillBuffer(AudioQueueRef queue, AudioQueueBufferRef buffer) {
        float* audioData = (float*)buffer->mAudioData;
        for (auto i = 0; i < BUFFER_SIZE / sizeof(float); i += 2) {
            float l = 0, r = 0;
            for (const auto& state: channels) {
                float position = beat - state.noteStart;
                float intensity = state.intensity;

                if (position < 0.1) {
                    intensity *= pow((position / 0.1), 3);
                }

                if (position > 0.95) {
                    intensity *= 1.0 - fmax(pow(((position - 0.95) / 0.05), 3), 0);
                }

                float raw = cos(position * 2 * M_PI * state.frequency);
                if (state.oscillator == Oscillator::Square) {
                    raw = (raw >= 0) - (raw <= 0);
                }
                if (state.oscillator == Oscillator::Noise) {
                    raw = random() / LONG_MAX;
                }
                float sample = raw * intensity;
                l += sample;
                r += sample;
            }
            audioData[i] = l / channels.size();
            audioData[i+1] = r / channels.size();
            tick();
        }

        AudioQueueEnqueueBuffer(queue, buffer, 0, NULL);
    }

private:
    void initializeAudio() {
        AudioStreamBasicDescription format;
        AudioQueueBufferRef buffers[3];

        format.mSampleRate       = SAMPLE_RATE;
        format.mFormatID         = kAudioFormatLinearPCM;
        format.mFormatFlags      = kLinearPCMFormatFlagIsFloat | kAudioFormatFlagIsPacked;
        format.mBitsPerChannel   = 8 * sizeof(SAMPLE_TYPE);
        format.mChannelsPerFrame = 2;
        format.mBytesPerFrame    = sizeof(SAMPLE_TYPE) * format.mChannelsPerFrame;
        format.mFramesPerPacket  = 1;
        format.mBytesPerPacket   = format.mBytesPerFrame * format.mFramesPerPacket;
        format.mReserved         = 0;

        AudioQueueNewOutput(&format, callback, this, NULL, kCFRunLoopCommonModes, 0, &queue);

        for (auto i = 0; i < NUM_BUFFERS; i++)
        {
            AudioQueueAllocateBuffer(queue, BUFFER_SIZE, &buffers[i]);

            buffers[i]->mAudioDataByteSize = BUFFER_SIZE;

            callback(this, queue, buffers[i]);
        }

        AudioQueueStart(queue, nullptr);
        CFRunLoopRun();
    }
};

struct Synthesizer {
    Song song;
    
};

