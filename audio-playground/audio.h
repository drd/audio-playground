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
#define BUFFER_SIZE 8192
#define SAMPLE_RATE 48000

extern "C" {
void synthPlay();
void synthStop();
void synthPause();
float* synthGetAudioData(int* numSamples);
}

float clamp(float input) {
    if (input < -1.0) {
        return -1.0;
    } else if (input > 1.0) {
        return 1.0;
    } else {
        return input;
    }
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
    return f;
}


struct NoteOn {
    Note note;
    uint32_t octave;
    float intensity;
    float length = 1.0f;
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
    float timeFactor = 1.0f;
};

struct Song {
    float tempo;
    std::vector<Pattern> patterns;
    std::vector<Channel> channels;
};

void callback(void *custom_data, AudioQueueRef queue, AudioQueueBufferRef buffer);

struct ChannelState {
    Oscillator oscillator;
    size_t channel = 0;
    size_t currentPattern = 0; // is an index into the index :)
    size_t currentNote = 0;
    float beat = 0;

    float timeFactor = 0;
    float phase = 0;
    float frequency = 0;
    float intensity = 0;

    float patternStart = 0.0f;
    float noteStart = 0.0f;
    float noteEnd = 0.0f;
    bool nextPattern = false;
    bool nextNote = false;
//    bool nextTick = false;
};

#define MAX_DELAY 120000
#define DELAY (12000)

struct Delay {
    std::array<float, MAX_DELAY> samples;
    size_t head = 0;
    float wet = 0.5;
    float dry = 0.5;
    float feedback = 0.5;

    void reset() {
        for (auto i = 0; i < MAX_DELAY; i++) {
            samples[i] = 0;
        }
    }

    void mix(float& left, float& right) {
        head %= DELAY;

        float output = samples[head];

        samples[head] = feedback * (output + left);

        left = left * dry + output * wet;
        right = right * dry + output * wet;

        head++;
    }

    void mix(float* buffer, size_t sampleCount) {
        for (auto i = 0; i < sampleCount; i += 2) {
            head %= DELAY;
            float output = samples[head];
            float input = feedback * (output + buffer[i]);
            buffer[i] = dry * buffer[i] + wet * output;
            buffer[i + 1] += dry * buffer[i + 1] + wet * output;
            samples[head] = input;
            head++;
        }
    }
};

struct Engine {
    size_t timeIndex = 0;
    float time;
    float beat;
    float* lastBuffer = nullptr;
    bool isPlaying = false;
    bool isPaused = false;
//    float note;
//    float tick;

    std::vector<ChannelState> channels;
    const Song& song;
    Delay delay;

    // System stuff
    AudioQueueRef queue;

    Engine(const Song& song)
    : song(song)
    {}

    void play() {
        if (isPlaying) {
            return;
        }

        if (isPaused) {
            isPaused = false;
            return;
        }

        isPlaying = true;
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

    void pause() {
        isPaused = !isPaused;
    }

    void stop() {
        isPlaying = false;
        isPaused = false;
        delay.reset();

        AudioQueueStop(queue, true);
        CFRunLoopStop(CFRunLoopGetCurrent());
        timeIndex = 0;
    }

    void tick() {
        timeIndex += 1;
        time = float(timeIndex) / SAMPLE_RATE;

        beat = time * song.tempo / 60.0;

        for (auto& state: channels) {
            auto c = song.channels[state.channel];
            auto p = song.patterns[c.patternIndex[state.currentPattern]];
            state.timeFactor = c.timeFactor;
            auto b = state.beat = beat * c.timeFactor;

            if (b - state.patternStart > p.notes.size()) {
                // next pattern!
                const auto& patternTable = song.channels[state.channel].patternIndex;
                state.currentPattern = (state.currentPattern + 1) % patternTable.size();
                state.nextNote = state.nextPattern = true;
                state.patternStart = floor(b);
                state.noteStart = time;
                state.phase = 0;
                p = song.patterns[state.currentPattern];
//                printf("t[%.3f] b[%.3f] New pattern: %zu\n", time, b, state.currentPattern);
            }

            auto note = floor(b - state.patternStart);
            const auto& n = p.notes[note];

            state.frequency = frequency(n.note, n.octave);
            state.intensity = n.intensity;
            state.phase += 2 * M_PI * state.frequency / SAMPLE_RATE;

            if (note != state.currentNote) {
                state.noteStart = time; // floor(b);
                state.currentNote = note;
                state.phase = 0;
            }
        }
    }

    void fillBuffer(AudioQueueRef queue, AudioQueueBufferRef buffer) {
        float* audioData = (float*)buffer->mAudioData;
        for (auto i = 0; i < BUFFER_SIZE; i += 2) {
            if (isPaused) {
                audioData[i] = audioData[i+1] = 0;
                delay.mix(audioData[i], audioData[i+1]);
                continue;
            }
            float l = 0, r = 0;
            for (const auto& state: channels) {
                float position = (time - state.noteStart) * song.tempo / 60.0f;
                float intensity = state.intensity;

                if (position < 0.1) {
                    float factor = pow((position / 0.1), 3);
                    intensity *= factor;
                }

                if (position > 0.95) {
                    float factor = 1.0 - fmax(pow(((position - 0.95) / 0.05), 3), 0);
                    intensity *= factor;
                }

                float raw = 0;
                float phase = state.phase; // (time - state.noteStart) * 2 * M_PI * state.frequency;
                if (state.oscillator == Oscillator::Sine) {
                    raw = sin(phase);
                } else if (state.oscillator == Oscillator::Square) {
                    float sine = cos(phase);
                    raw = (sine >= 0) - (sine <= 0);
                } else if (state.oscillator == Oscillator::Noise) {
                    float sine = cos(phase);
                    raw = (sine >= 0) - (sine <= 0);
                }
                float sample = raw * intensity;
                l += sample;
                r += sample;
            }

            float ml = l / channels.size();
            float mr = r / channels.size();

            delay.mix(ml, mr);

            audioData[i] = ml;
            audioData[i+1] = mr;
            tick();
        }

//        delay.mix(audioData, BUFFER_SIZE);
        lastBuffer = audioData;

        AudioQueueEnqueueBuffer(queue, buffer, 0, NULL);
    }

private:
    void initializeAudio() {
        AudioStreamBasicDescription format;
        AudioQueueBufferRef buffers[NUM_BUFFERS];

        format.mSampleRate       = SAMPLE_RATE;
        format.mFormatID         = kAudioFormatLinearPCM;
        format.mFormatFlags      = kLinearPCMFormatFlagIsFloat | kAudioFormatFlagIsPacked;
        format.mBitsPerChannel   = 8 * sizeof(float);
        format.mChannelsPerFrame = 2;
        format.mBytesPerFrame    = sizeof(float) * format.mChannelsPerFrame;
        format.mFramesPerPacket  = 1;
        format.mBytesPerPacket   = format.mBytesPerFrame * format.mFramesPerPacket;
        format.mReserved         = 0;

        AudioQueueNewOutput(&format, callback, this, NULL, kCFRunLoopCommonModes, 0, &queue);

        for (auto i = 0; i < NUM_BUFFERS; i++)
        {
            AudioQueueAllocateBuffer(queue, BUFFER_SIZE * sizeof(float), &buffers[i]);

            buffers[i]->mAudioDataByteSize = BUFFER_SIZE * sizeof(float);

            callback(this, queue, buffers[i]);
        }

        AudioQueueStart(queue, nullptr);
        CFRunLoopRun();
    }
};

struct Synthesizer {
    Song song;
};

