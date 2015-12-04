#include "audio.h"
#include <windows.h>
#include <winerror.h>
#include <dsound.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/* Somehow, Digital Mars doesn't get this definition. Probably
 * because of their poor quality Windows.h and friends it insists
 * on using rather than the MS or Cygwin versions.
 */
#ifndef FAILED
#define FAILED(X) (X<0)
#endif

/* Some defaults. Ideally some of these would be modifiable.
 *
 * The ATHENA_PACKET* are just suggestions to reduce allocations.
 * Things _should_ still work no matter the values, so long as they
 * are all > 0 && < 0xFFFFFFFF
 */
#define ATHENA_CHANNELS 2
#define ATHENA_RATE 44100
#define ATHENA_SAMPLE_BITS 16

/* This is the default packet size we expect. */
#define ATHENA_PACKET_SIZE 5760
/* Expected length of packets in milliseconds */
#define ATHENA_PACKET_MS 60
/* Default number of secondary buffers to create. */
#define ATHENA_BUFFERS 2

struct Athena_SoundContext{
    LPDIRECTSOUND8 ctx;
    LPDIRECTSOUNDBUFFER primary_buffer;
};

static const char *athena_ds_err(HRESULT err){
    switch(err){
        case DS_OK:
            return "OK";
        case DSERR_GENERIC:
            return "Unspecified Error";
        case DSERR_CONTROLUNAVAIL:
            return "Control not available";
        case DSERR_INVALIDPARAM:
            return "Bad Parameter";
        case DSERR_PRIOLEVELNEEDED:
            return "Priority level needed";
        default:
            printf("[FIXME] Unhandled error %i\n", (int)err);
            return "[FIXME] Unknown";
    }
}

struct Athena_Sound{
    struct Athena_SoundContext *ctx;

    WAVEFORMATEX wave_format;

    LPDIRECTSOUNDBUFFER buffer[ATHENA_BUFFERS];
    unsigned size, num_buffers;
    void *soft_buffer;
    /* All in bytes, regardless of bits per sample */
    unsigned soft_size, soft_capacity, soft_cursor;

    struct Athena_SoundConfig config;
};

/*
struct Athena_SoundConfig{
    float volume, pan;
    unsigned char loop;
};
*/
enum Athena_SoundFormat Athena_PreferredFormat(struct Athena_SoundContext *ctx){
    return Athena_SoundU16;
}

enum Athena_SoundFormat Athena_PreferredSoundFormat(struct Athena_Sound *snd){
    return Athena_SoundU16;
}

unsigned Athena_SupportsFormat(struct Athena_SoundContext *ctx, enum Athena_SoundFormat format){
    return format==Athena_SoundU16 || format==Athena_SoundU32;
}

static void *athena_release_ctx(struct Athena_SoundContext *ctx){
    if(ctx->ctx)
        IDirectSound_Release(ctx->ctx);
    if(ctx->primary_buffer)
        IDirectSoundBuffer_Release(ctx->primary_buffer);
    return NULL;
}

static void athena_wave_format_prepare(WAVEFORMATEX *wave_format, unsigned channels, unsigned samples_per_second, unsigned bits_per_sample){

    memset(wave_format, 0, sizeof(WAVEFORMATEX));

    wave_format->nChannels = ATHENA_CHANNELS;
    wave_format->nSamplesPerSec = ATHENA_RATE;
    wave_format->wBitsPerSample = ATHENA_SAMPLE_BITS;

    wave_format->wFormatTag = WAVE_FORMAT_PCM;
    wave_format->nBlockAlign = /* BYTES per sample times number of channels. */
        (wave_format->wBitsPerSample >> 3) * wave_format->nChannels;

    wave_format->nAvgBytesPerSec =wave_format->nSamplesPerSec * wave_format->nBlockAlign;
}

struct Athena_SoundContext *Athena_CreateSoundContext(){
    struct Athena_SoundContext l_ctx;
    HRESULT err;
    memset(&l_ctx, 0, sizeof(struct Athena_SoundContext));

    if(FAILED(DirectSoundCreate8(NULL, &l_ctx.ctx, NULL)))
        return athena_release_ctx(&l_ctx);
    else{

        DSBUFFERDESC buffer_info;
        WAVEFORMATEX wave_format;

        memset(&buffer_info, 0, sizeof(DSBUFFERDESC));

        /* This isn't the best, but it is the simplest. */
        if(FAILED(err = IDirectSound_SetCooperativeLevel(l_ctx.ctx, GetDesktopWindow(), DSSCL_PRIORITY))){
           fprintf(stderr, "Cannot set cooperative level.\nError Code %s\n", athena_ds_err(err));
        }
        buffer_info.dwSize = sizeof(DSBUFFERDESC);
        buffer_info.dwFlags = DSBCAPS_PRIMARYBUFFER;

        if(FAILED(err = IDirectSound_CreateSoundBuffer(l_ctx.ctx, &buffer_info, &l_ctx.primary_buffer, NULL))){
            fprintf(stderr, "Cannot create primary buffer.\nError Code %s\n", athena_ds_err(err));
            return athena_release_ctx(&l_ctx);
        }
        athena_wave_format_prepare(&wave_format, ATHENA_CHANNELS, ATHENA_RATE, ATHENA_SAMPLE_BITS);

        if(FAILED(IDirectSoundBuffer_SetFormat(l_ctx.primary_buffer, &wave_format))){
            return athena_release_ctx(&l_ctx);
        }
        else{
            struct Athena_SoundContext *ctx = malloc(sizeof(struct Athena_SoundContext));
            assert(ctx);

            ctx[0] = l_ctx;
            return ctx;
        }
    }
}

void Athena_DestroySoundContext(struct Athena_SoundContext *ctx){
    assert(ctx);
    athena_release_ctx(ctx);
    free(ctx);
}

struct Athena_Sound *Athena_CreateSound(struct Athena_SoundContext *ctx){
    struct Athena_Sound *const snd = malloc(sizeof(struct Athena_Sound));

    memset(snd, 0, sizeof(struct Athena_Sound));
    snd->ctx = ctx;
    /* Default to the size of a single Opus packet. There are few decoders,
     * if any, that have a larger default packet size than Opus. */
    snd->soft_buffer = malloc(
        snd->soft_capacity = ATHENA_PACKET_SIZE * (ATHENA_SAMPLE_BITS >> 3) * ATHENA_CHANNELS
    );

    snd->config.volume = snd->config.pan = 0.0f;

    return snd;
}

void Athena_DestroySound(struct Athena_Sound *snd){
    if(snd->soft_buffer)
        free(snd->soft_buffer);
}

void Athena_SoundInit(struct Athena_Sound *snd, unsigned num_channels, unsigned samples_per_second, enum Athena_SoundFormat format){
    DSBUFFERDESC buffer_info;

    memset(&buffer_info, 0, sizeof(DSBUFFERDESC));

    athena_wave_format_prepare(&snd->wave_format, num_channels, samples_per_second, (format==Athena_SoundU32)?32:16);

#if ATHENA_PACKET_MS < 1000
    buffer_info.dwBufferBytes = ATHENA_PACKET_SIZE * (1000 / ATHENA_PACKET_MS);
#else
    buffer_info.dwBufferBytes = ATHENA_PACKET_SIZE *  16;
#endif
    if(buffer_info.dwBufferBytes > DSBSIZE_MAX){
        fprintf(stderr, "[Athena_SoundInit]Warning: Clamping buffer size from %i to %i\n", buffer_info.dwBufferBytes, DSBSIZE_MAX);
        buffer_info.dwBufferBytes = DSBSIZE_MAX;
    }
    if(buffer_info.dwBufferBytes < DSBSIZE_MIN){
        fprintf(stderr, "[Athena_SoundInit]Warning: Clamping buffer size from %i to %i\n", buffer_info.dwBufferBytes, DSBSIZE_MIN);
        buffer_info.dwBufferBytes = DSBSIZE_MIN;
    }

    buffer_info.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME;
    buffer_info.dwSize = sizeof(DSBUFFERDESC);

    buffer_info.lpwfxFormat = &snd->wave_format;

    {
        int i = 0;
        HRESULT err;
        while(i<ATHENA_BUFFERS){
            if(FAILED(err = IDirectSound_CreateSoundBuffer(snd->ctx->ctx, &buffer_info, snd->buffer + i, NULL))){
                fputs("Failed to create sound buffer\n", stderr);
                fprintf(stderr, "Error code %s\n", athena_ds_err(err));
            }
            i++;
        }
    }

}

void Athena_SoundGetConfig(const struct Athena_Sound *snd, struct Athena_SoundConfig *to){
    to[0] = snd->config;
}

void Athena_SoundSetConfig(struct Athena_Sound *snd, const struct Athena_SoundConfig *to){
    int i = 0;
    const LONG pan = (to->pan==0.0f) ? DSBPAN_CENTER :
        (to->pan>0.0f) ? to->pan*DSBPAN_RIGHT : - to->pan * DSBPAN_LEFT;

    snd->config = *to;

    while(i<ATHENA_BUFFERS){
        if(snd->buffer[i]){
            IDirectSoundBuffer_SetVolume(snd->buffer[i], snd->config.volume * DSBVOLUME_MAX);
            IDirectSoundBuffer_SetPan(snd->buffer[i], pan);
        }
        i++;
    }
}

float Athena_SoundGetLength(const struct Athena_Sound *snd){
    return ((float)snd->soft_size) / ((float)snd->wave_format.nAvgBytesPerSec);
}

unsigned Athena_SoundGetChannels(const struct Athena_Sound *snd){
    return snd->wave_format.nChannels;
}

unsigned Athena_SoundGetSamplesPerSecond(const struct Athena_Sound *snd){
    return snd->wave_format.nSamplesPerSec;
}

enum Athena_SoundFormat Athena_SoundGetFormat(const struct Athena_Sound *snd){
    return (snd->wave_format.wBitsPerSample==32) ? Athena_SoundU32 : Athena_SoundU16;
}

unsigned Athena_SoundPost(struct Athena_Sound *snd, const void *data, unsigned length){
#define TOO_SMALL(X_, E_) (X_->soft_capacity < X_->soft_size + E_)

    if(TOO_SMALL(snd, length)){
        while(TOO_SMALL(snd, length))
            snd->soft_capacity <<= 1;
        snd->soft_buffer = realloc(snd->soft_buffer, snd->soft_capacity);
    }

    assert(!TOO_SMALL(snd, length));

#undef TOO_SMALL

    memcpy(((uint8_t*)snd->soft_buffer) + snd->soft_size, data, length);
    snd->soft_size += length;

    return 0;

}

void Athena_SoundPlay(struct Athena_Sound *sound){

}

void Athena_SoundPause(struct Athena_Sound *sound){

}

void Athena_SoundStop(struct Athena_Sound *sound){

}

void Athena_SoundRewind(struct Athena_Sound *sound){

}
