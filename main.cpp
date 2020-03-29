#include <SDL2/SDL.h>
#include "notes.hpp"

const float PI = 3.14159274101f;

void sec(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError());
        abort();
    }
}

template <typename T>
T *sec(T *ptr)
{
    if (ptr == nullptr) {
        fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError());
        abort();
    }

    return ptr;
}

struct Note
{
    float freq;
    float duration;
};

struct Melody
{
    size_t notes_size;
    Note *notes;
    float t;

    float current_db() const
    {
        size_t i = 0;
        float ti = 0.0f;
        for (; i < notes_size; ++i)
        {
            if (ti + notes[i].duration > t) break;
            ti += notes[i].duration;
        }

        const Note note = i < notes_size ? notes[i] : Note {0.0f, 0.0f};
        const float delta_angle = 2 * PI * note.freq;
        const float attacc = 32.0f;
        const float decay = 16.0f;
        const float volume = fminf(1.0f, ((t - ti) / notes[i].duration) * attacc)
                              * fminf(1.0f, (1.0f - (t - ti) / notes[i].duration) * decay);
        return sinf(t * delta_angle) * volume;
    }

    float duration() const
    {
        float result = 0.0f;
        for (size_t i = 0; i < notes_size; ++i) {
            result += notes[i].duration;
        }
        return result;
    }
};

const int BPM = 130;


#define Dur(n) (float) ((4.0f * (1.0f / (BPM / 60.0))) / (float) (n))

Note megalovania[] = {
    {D3,  Dur(16)},
    {D3,  Dur(16)},
    {D4,  Dur(8)},
    {A3,  Dur(8)},
    {P,   Dur(16)},
    {GS3, Dur(16)},
    {P,   Dur(16)},
    {G3,  Dur(16)},
    {P,   Dur(16)},
    {F3,  Dur(8)},
    {D3,  Dur(16)},
    {F3,  Dur(16)},
    {G3,  Dur(16)},
};
size_t megalovania_count = sizeof(megalovania) / sizeof(megalovania[0]);

const int SOMETHING_AUDIO_FREQUENCY = 48000;
const int SOMETHING_AUDIO_SAMPLES = 4096;
const float AUDIO_CALLBACK_DT =
    (float) SOMETHING_AUDIO_SAMPLES / (float) SOMETHING_AUDIO_FREQUENCY;
const float AUDIO_SAMPLE_DT =
    AUDIO_CALLBACK_DT / (float) SOMETHING_AUDIO_SAMPLES;

void MelodyAudioCallback(void *userdata, Uint8 *stream, int len)
{
    Melody *melody = (Melody*) userdata;

    float *output = (float*)stream;
    int output_len = len / (int) sizeof(*output);

    for (int i = 0; i < output_len; i += 2) {
        output[i + 1] = output[i] = melody->current_db();
        melody->t = fmodf(melody->t + AUDIO_SAMPLE_DT, melody->duration());
    }
}

int main(void)
{
    sec(SDL_Init(SDL_INIT_AUDIO));

    Melody melody = {};
    melody.notes = megalovania;
    melody.notes_size = megalovania_count;

    SDL_AudioSpec want = {};
    want.freq = SOMETHING_AUDIO_FREQUENCY;
    want.format = AUDIO_F32;
    want.channels = 2;
    want.samples = SOMETHING_AUDIO_SAMPLES;
    want.callback = MelodyAudioCallback;
    want.userdata = &melody;

    SDL_AudioSpec have = {};
    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(
        NULL,
        0,
        &want,
        &have,
        SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (dev == 0) {
        fprintf(stderr, "SDL pooped itself: Failed to open audio: %s\n", SDL_GetError());
        abort();
    }

    if (have.format != want.format) {
        fprintf(stderr, "[WARN] We didn't get Float32 audio format.\n");
    }

    SDL_PauseAudioDevice(dev, 0);
    SDL_Delay(10000);
    SDL_CloseAudioDevice(dev);

    SDL_Quit();

    return 0;
}
