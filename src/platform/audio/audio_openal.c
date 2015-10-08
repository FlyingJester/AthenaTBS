#include "audio.h"
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__

    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>

#else

    #include <AL/al.h>
    #include <AL/alext.h>
    #include <AL/alc.h>

#endif

/*
struct Athena_SoundConfig{
    float volume, pan;
    unsigned char loop;
};

enum Athena_SoundFormat { Athena_SoundU16, Athena_SoundU32, Athena_SoundFloat };
*/

#define DEFAULT_STARTING_BUFFERS 2

struct Athena_SoundContext{
    ALCdevice *device;
    ALCcontext *context;
};

struct Athena_Sound{
    struct Athena_SoundConfig config;
    struct Athena_SoundContext *context;

    unsigned format, num_channels, samples_per_second;
    float length;    

    ALuint *buffers;
    unsigned num_buffers, used_buffers;
    
    ALuint source;
};

static void athena_make_context_current(const struct Athena_SoundContext *ctx){
    if(ctx)
        alcMakeContextCurrent(ctx->context);
}

struct Athena_SoundContext *Athena_CreateSoundContext(){
    ALCdevice *device;
    ALCcontext *context;
    
    if(!(device = alcOpenDevice(NULL))){
        return NULL;
    }
    
    if(!(context = alcCreateContext(device, NULL)))
        return NULL;
    
    alcMakeContextCurrent(context);
    
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alListener3f(AL_ORIENTATION, 0.0f, 0.0f, 0.0f);

    {
        struct Athena_SoundContext * const r_ctx = calloc(sizeof(struct Athena_SoundContext), 1);
        r_ctx->context = context;
        r_ctx->device = device;
        return r_ctx;
    }
}

void Athena_DestroySoundContext(struct Athena_SoundContext *ctx){
    if(!ctx)
        return;

    alcCloseDevice(ctx->device);

    free(ctx);
}

enum Athena_SoundFormat Athena_PreferredFormat(struct Athena_SoundContext *ctx){
    return Athena_SoundU16;
}

unsigned Athena_SupportsFormat(struct Athena_SoundContext *ctx, enum Athena_SoundFormat format){
    return Athena_PreferredFormat(ctx)==format;
}

struct Athena_Sound *Athena_CreateSound(struct Athena_SoundContext *ctx){
    struct Athena_Sound * const sound = calloc(sizeof(struct Athena_Sound), 1);
    sound->context = ctx;

    athena_make_context_current(ctx);

    sound->num_buffers = DEFAULT_STARTING_BUFFERS;
    if(!sound->num_buffers)
        sound->num_buffers = 1;
    sound->buffers = calloc(sizeof(ALuint), sound->num_buffers);

    alGenBuffers(sound->num_buffers, sound->buffers);
    alGenSources(1, &sound->source);
    
    return sound;

}

void Athena_DestroySound(struct Athena_Sound *sound){
    
    athena_make_context_current(sound->context);

    alSourceStop(sound->source);
    alDeleteBuffers(sound->num_buffers, sound->buffers);
    alDeleteSources(1, &sound->source);

    free(sound->buffers);
    free(sound);
}

void Athena_SoundInit(struct Athena_Sound *sound, unsigned num_channels, unsigned samples_per_second, enum Athena_SoundFormat format){

    if(num_channels==2)
        sound->format = AL_FORMAT_STEREO16;
    else
        sound->format = AL_FORMAT_MONO16;

    sound->samples_per_second = samples_per_second;

}

void Athena_SoundGetConfig(const struct Athena_Sound *sound, struct Athena_SoundConfig *to){
    to[0] = sound->config;
}

void Athena_SoundSetConfig(struct Athena_Sound *sound, const struct Athena_SoundConfig *to){

    if(fabs(sound->config.volume - to->volume) > 0.01f){
        alSourcef(sound->source, AL_GAIN, to->volume);
        sound->config.volume = to->volume;
    }

    if(fabs(sound->config.pan - to->pan) > 0.01f){
        sound->config.pan = to->pan;
    }
    
    if(sound->config.loop != to->loop){
        alSourcei(sound->source, AL_LOOPING, to->loop?AL_TRUE:AL_FALSE);
        sound->config.loop = to->loop;
    }

}

float Athena_SoundGetLength(const struct Athena_Sound *sound){
    return sound->length;
}

unsigned Athena_SoundGetChannels(const struct Athena_Sound *sound){
    return sound->num_channels;
}

unsigned Athena_SoundGetSamplesPerSecond(const struct Athena_Sound *sound){
    return sound->samples_per_second;
}

enum Athena_SoundFormat Athena_SoundGetFormat(const struct Athena_Sound *sound){
    return Athena_PreferredFormat(sound->context);
}

const void *Athena_SoundGetContext(const struct Athena_Sound *sound){
    return sound->context->context;
}

static int athena_find_matching_buffer_index(ALuint inner, unsigned max, unsigned i, const ALuint *buffers){
    if(i==max)
        return -1;
    else if(buffers[i]==inner)
        return i;
    else
        return athena_find_matching_buffer_index(inner, max, i+1, buffers);
}

static void athena_return_used_buffers(struct Athena_Sound *sound, unsigned num_buffers){
    if(num_buffers && sound->used_buffers){
        ALuint temp_buffer;
        alSourceUnqueueBuffers(sound->source, 1, &temp_buffer);
        {
            const int at = athena_find_matching_buffer_index(temp_buffer, sound->num_buffers, 0, sound->buffers);
            /* This really should be true, but we check just in case. */
            if(at < sound->used_buffers){
                /* We swap the now open buffer to the position that was previously the last used buffer, and decrease the number of used buffers. */
                sound->used_buffers--;
                sound->buffers[at] = sound->buffers[sound->used_buffers];
                sound->buffers[sound->used_buffers] = temp_buffer;
            }
        }
        athena_return_used_buffers(sound, num_buffers-1);
    }
}

unsigned Athena_SoundPost(struct Athena_Sound *sound, const void *data, unsigned length){
    athena_make_context_current(sound->context);
    
    /* Unlike Cinnamon, we always try to return buffers when new data is posted. */
    if(sound->used_buffers){
        ALint num_buffers;
        alGetSourcei(sound->source, AL_BUFFERS_PROCESSED, &num_buffers);
        athena_return_used_buffers(sound, num_buffers);
    }

    /* Very unlikely, but in case we really have no free buffers, we allocate some more. */
    while(!(sound->used_buffers<sound->num_buffers)){
        sound->buffers = realloc(sound->buffers, sound->num_buffers << 1);

        alGenBuffers(sound->num_buffers, sound->buffers + sound->num_buffers);

        sound->num_buffers<<=1;
    }

    alBufferData(sound->buffers[sound->used_buffers], sound->format, data, length, sound->samples_per_second);
    alSourceQueueBuffers(sound->source, 1, sound->buffers + sound->used_buffers);

    sound->used_buffers++;
    
    return 0;
}

void Athena_SoundPlay(struct Athena_Sound *sound){
    alSourcePlay(sound->source);
}

void Athena_SoundPause(struct Athena_Sound *sound){
    alSourcePause(sound->source);
}

void Athena_SoundStop(struct Athena_Sound *sound){
    alSourceStop(sound->source);
}

void Athena_SoundRewind(struct Athena_Sound *sound){
    alSourceRewind(sound->source);
}

