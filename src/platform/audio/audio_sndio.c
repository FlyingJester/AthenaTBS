#include "audio.h"
#include <sndio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>

/* All data is stored per-sound for the sndio backend. */
struct Athena_SoundContext { int x; };

/* We need to return non-null on ctx creation to indicate success.
 * This also could hold the libswr context, should we use it.
 */

static struct Athena_SoundContext global_context; 

struct Athena_Sound{
    struct sio_hdl *handle;
    struct sio_par params;
    
    enum Athena_SoundFormat informat, outformat;

    /* Sizes are in samples */
    unsigned buffer_size, index, inchans, outchans;
    void *buffer;
    
    struct Athena_SoundConfig config;
};

static void athena_sndio_write(struct Athena_Sound *snd, unsigned amount){
    assert(snd->buffer_size - snd->index >= amount);

    if(snd->informat == snd->outformat){
        sio_write(snd->handle, (unsigned char *)snd->buffer + snd->index, amount);
    }
    else{
        void *outbuffer = calloc(ATHENA_FORMAT_BYTE_SIZE(snd->outformat) * amount);
    }
}

static void athena_sndio_callback(struct Athena_Sound *snd, unsigned a_amount){
    const unsigned left = snd->buffer_size - snd->index;
    const unsigned amount = min(left, a_amount);
    const unsigned next = a_amount - amount;

    assert(a_amount >= amount);

    /* Called twice to avoid the jump from the end of the buffer to the start */
    athena_sndio_write(snd, amount);
    if(next && snd->config.loop)
        athena_sndio_write(snd, next * ATHENA_FORMAT_BYTE_SIZE(snd->outformat));
}

enum Athena_SoundFormat Athena_PreferredFormat(struct Athena_SoundContext *ctx){
    assert(ctx == &global_context);
    return Athena_SoundS16;
}

enum Athena_SoundFormat Athena_PreferredSoundFormat(struct Athena_Sound *snd){
    return Athena_PreferredFormat(&global_context);
}

unsigned Athena_SupportsFormat(struct Athena_SoundContext *ctx, enum Athena_SoundFormat format){
    assert(ctx == &global_context);
    return format != Athena_SoundFloat;
}

struct Athena_SoundContext *Athena_CreateSoundContext(){
    return &global_context;
}

void Athena_DestroySoundContext(struct Athena_SoundContext *ctx){
    assert(ctx == &global_context);
}

struct Athena_Sound *Athena_CreateSound(struct Athena_SoundContext *ctx){
    struct sio_hdl *handle;
    assert(ctx == &global_context);
    if(!(handle = sio_open(SIO_DEVANY, SIO_PLAY, 0))){
        return NULL;
    }
    else{
        struct Athena_Sound *const snd = (struct Athena_Sound *)calloc(1, sizeof(struct Athena_Sound));

        snd->handle = handle;
        sio_initpar(&snd->params);
        snd->params.xrun = SIO_IGNORE;

        sio_setvol(handle, snd->config.volume = SIO_MAXVOL);

        return snd;
    }
}

void Athena_DestroySound(struct Athena_Sound *snd){
    assert(snd);
    if(!snd)
        return;

    sio_close(snd->handle);
    
    free(snd->buffer);
    
}

enum Athena_SoundFormat Athena_SoundGetFormat(const struct Athena_Sound *snd){
    assert(snd);
    if(snd){
#ifndef NDEBUG
        const unsigned snd_bits = snd->params.bits;
        assert(snd_bits == 8 || snd_bits == 16 || snd_bits == 24 || snd_bits == 32);
#endif
        switch(snd->params.bits){
            case 8:
                return Athena_SoundU8;
            case 16:
                return Athena_SoundS16;
            case 24:
                return Athena_SoundS24;
            case 32:
                return Athena_SoundS32;
        }
    }
    return 0;
}

/*
struct Athena_SoundConfig{
    float volume, pan;
    unsigned char loop;
};
*/

void Athena_SoundInit(struct Athena_Sound *snd, unsigned num_channels, unsigned samples_per_second, enum Athena_SoundFormat format){
    unsigned channels, bits, endian;
#define RESET_FLAGS if((channels = 0) || (bits = 0) || (endian = 0)){}

    RESET_FLAGS;

    assert(format != Athena_SoundFloat);

    snd->inchans = num_channels;

/* not even worth trying without OpenBSD as our backend. */
#ifdef __linux__
    if(num_channels < 2)
        num_channels = 2;
#endif

/* Try the parameters. */
    snd->params.pchan = num_channels;
    channels = 1;

no_channels:
    snd->params.bits = 8 * ATHENA_FORMAT_BYTE_SIZE(format);
    snd->params.bps = SIO_BPS(snd->params.bits);
    bits = 1;

no_bits:
    snd->params.sig = 1;
    snd->params.le = SIO_LE_NATIVE;
    endian = 1;

no_endian:
    snd->params.rate = samples_per_second;
    snd->params.xrun = SIO_IGNORE;

    sio_setpar(snd->handle, &snd->params);
    sio_getpar(snd->handle, &snd->params);
    
    if(channels){
        if(snd->params.pchan != num_channels){
            RESET_FLAGS;
            goto no_channels;
        }
    }
    else if(bits){
        if(snd->params.bits != 8 * ATHENA_FORMAT_BYTE_SIZE(format)){
            RESET_FLAGS;
            goto no_bits;
        }
    }
    else if(endian){
        if(snd->params.sig != 1 || snd->params.le != SIO_LE_NATIVE){
            RESET_FLAGS;
            goto no_endian;
        }
    }
    /* Otherwise, this is just the shape we're in. */

    if(snd->params.rate != samples_per_second)
        fprintf(stderr, 
            "Frequency mismatch. Requested %i, got %i.\n"\
            "\tWe will still play back sound, but it will be at the wrong speed.\n",
            samples_per_second, snd->params.rate);

#undef RESET_FLAGS
}

void Athena_SoundGetConfig(const struct Athena_Sound *snd, struct Athena_SoundConfig *to){
    assert(snd);
    assert(to);
    
    to[0] = snd->config;
    
}

void Athena_SoundSetConfig(struct Athena_Sound *snd, const struct Athena_SoundConfig *to){
    if(abs(to->volume - snd->config.volume) > ATHENA_VOLUME_EPSILON)
        sio_setvol(snd->handle, to->volume * (float)SIO_MAXVOL);
    snd->config = to[0];
}
