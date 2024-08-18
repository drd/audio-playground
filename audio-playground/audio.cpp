//
//  audio.c
//  audio-playground
//
//  Created by Eric O'Connell on 8/17/24.
//

#include "audio.h"

std::vector<Pattern> patterns = {
    {
        .notes = {
            { Note::A, 3, 0.7f },
            { Note::B, 3, 0.7f },
            { Note::C, 3, 0.7f },
            { Note::A, 3, 0.7f },
        }
    },
    {
        .notes = {
            { Note::G, 3, 0.7f },
            { Note::F, 3, 0.5f },
            { Note::E, 3, 0.6f },
            { Note::F, 3, 0.5f },
            { Note::E, 3, 0.6f },
        }
    },
    {
        .notes = {
            { Note::E, 3, 0.6f },
            { Note::F, 3, 0.5f },
            { Note::A, 3, 0.7f },
            { Note::G, 3, 0.5f },
            { Note::C, 3, 0.6f },
            { Note::C, 3, 0.7f },
            { Note::A, 3, 0.7f },
        }
    },
    {
        .notes = {
            { Note::E, 4, 0.6f },
            { Note::E, 4, 0.6f },
            { Note::F, 4, 0.5f },
            { Note::A, 5, 0.7f },
            { Note::A$, 5, 0.7f },
//            { Note::G, 3, 0.5f },
//            { Note::C, 3, 0.6f },
//            { Note::C, 3, 0.7f },
//            { Note::A, 3, 0.7f },
        }
    },
};

std::vector<Channel> channels = {
    {
        .patternIndex = {0, 1, 2},
        .oscillator = Oscillator::Sine
    },
    {
        .patternIndex = {2, 0, 1},
        .oscillator = Oscillator::Square
    },
    {
        .patternIndex = {3},
        .oscillator = Oscillator::Sine
    }
};

Song song = {
    .patterns = patterns,
    .channels = channels
};

Engine engine = { song };
//unsigned long count = 0;

void play() {
    engine.play();

//    AudioStreamBasicDescription format;
//    AudioQueueBufferRef buffers[3];
//    AudioQueueRef queue;
//
//    format.mSampleRate       = SAMPLE_RATE;
//    format.mFormatID         = kAudioFormatLinearPCM;
//    format.mFormatFlags      = kLinearPCMFormatFlagIsFloat | kAudioFormatFlagIsPacked;
//    format.mBitsPerChannel   = 8 * sizeof(SAMPLE_TYPE);
//    format.mChannelsPerFrame = 2;
//    format.mBytesPerFrame    = sizeof(SAMPLE_TYPE) * format.mChannelsPerFrame;
//    format.mFramesPerPacket  = 1;
//    format.mBytesPerPacket   = format.mBytesPerFrame * format.mFramesPerPacket;
//    format.mReserved         = 0;
//
//    AudioQueueNewOutput(&format, callback, nullptr, nullptr, kCFRunLoopCommonModes, 0, &queue);
//
//    for (auto i = 0; i < NUM_BUFFERS; i++)
//    {
//        AudioQueueAllocateBuffer(queue, BUFFER_SIZE, &buffers[i]);
//
//        buffers[i]->mAudioDataByteSize = BUFFER_SIZE;
//
//        callback(nullptr, queue, buffers[i]);
//    }
//
//    AudioQueueStart(queue, NULL);
//    CFRunLoopRun();
}

void stop() {
    engine.stop();
}

void callback(void *custom_data, AudioQueueRef queue, AudioQueueBufferRef buffer) {
    auto engine = static_cast<Engine*>(custom_data);
    engine->fillBuffer(queue, buffer);

}

//void callback(void *custom_data, AudioQueueRef queue, AudioQueueBufferRef buffer)
//{
////    printf("Buffer: %p Count: %zu\n", buffer, count);
//    SAMPLE_TYPE *casted_buffer = (SAMPLE_TYPE *)buffer->mAudioData;
//
//    unsigned int i;
//
//    float base = 440.0f;
//    float fs[] = {
//        base,
//        base * logf(3.0f) / logf(12.0f),
//        base * logf(5.0f) / logf(12.0f),
//        base * logf(7.0f) / logf(12.0f),
//        base * logf(10.0f) / logf(12.0f),
//    };
//
//    float base2 = base * logf(2.0f) / logf(12.0f);
//    float f2s[] = {
//        base2 * logf(5.0f) / logf(12.0f),
//        base2,
//        base2 * logf(7.0f) / logf(12.0f),
//        base2 * logf(3.0f) / logf(12.0f),
//        base2 * logf(10.0f) / logf(12.0f),
//    };
//
//    for (i = 0; i < BUFFER_SIZE / sizeof(SAMPLE_TYPE); i += 2)
//    {
//        float c1r, c1l;
//        float c2r, c2l;
//        float c3r, c3l;
//
//        {
//            auto time = float(count) / float(SAMPLE_RATE);
//            auto beat = time * 120.0 / 60.0;
//            auto note = fmod(beat / 4.0, 1.0) * 4;
//
//            auto subnote = fmod(note, 1.0);
//
//            auto freq = fs[int(floor(note))] * 2;
//
//            float sample = cos(time * 2 * M_PI * freq);
//            float amplitude = (1.0 - subnote) * cos(subnote * M_PI_2);
//
//            auto output = sample * amplitude;
//            double pan = 0.75 + 0.25 * sin(count / 11115.0);
//
//            c1l = pan * output;
//            c1r = (1.0 - pan) * output;
//        }
//
//
//        {
//            auto time = float(count) / float(SAMPLE_RATE);
//            auto beat = time * 120.0 / 60.0;
//            auto note = fmod(beat / 3.0, 1.0) * 5;
//
//            auto subnote = fmod(note, 1.0);
//
//            auto freq = f2s[int(floor(note))] * 2;
//
//
//
//            float sample = cos(time * 2 * M_PI * freq);
//            float amplitude = (1.0 - subnote) * cos(subnote * M_PI_2);
//
//            auto output = sample * amplitude;
//            double pan = 0.66 + 0.3 * sin(count / 21115.0);
//
//            c2l = pan * output;
//            c2r = (1.0 - pan) * output;
//        }
//
//
//        casted_buffer[i]   = 0.6 * c1l + 0.4 * c2l;
//        casted_buffer[i+1] = 0.6 * c1r + 0.4 * c2r;
//
//        count++;
//    }
//
//    AudioQueueEnqueueBuffer(queue, buffer, 0, NULL);
//
//    if (count > SAMPLE_RATE * 1000)
//    {
//        AudioQueueStop(queue, false);
//        AudioQueueDispose(queue, false);
//        CFRunLoopStop(CFRunLoopGetCurrent());
//    }
//
//}
