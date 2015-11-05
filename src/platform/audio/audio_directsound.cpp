#include "audio.h"

/*
	LPVOID m_pTheSound;

	DWORD m_dwTheSound;

	LPDIRECTSOUNDBUFFER m_pDsb;

	BOOL m_bEnabled;

	static LPDIRECTSOUND m_lpDirectSound;

	static DWORD m_dwInstances;
*/

struct Athena_SoundContext{
    LPDIRECTSOUND direct_sound;
};

struct Athena_Sound{
    struct Athena_SoundContext ctx;
    WAVEFORMATEX wfx;
    LPDIRECTSOUNDBUFFER buffer;
}

struct Athena_SoundConfig{
    float volume, pan;
    unsigned char loop;
};

enum Athena_SoundFormat { Athena_SoundU16, Athena_SoundU32, Athena_SoundFloat };

/* This MUST be either 16-bit integer or 32-bit float. */
enum Athena_SoundFormat Athena_PreferredFormat(struct Athena_SoundContext *ctx){
    return Athena_SoundU16;
}

enum Athena_SoundFormat Athena_PreferredSoundFormat(struct Athena_Sound *snd){
    return Athena_SoundU16;
}

unsigned Athena_SupportsFormat(struct Athena_SoundContext *, enum Athena_SoundFormat){
    return format==Athena_SoundU16 || format==Athena_SoundU32;
}

struct Athena_SoundContext *Athena_CreateSoundContext(){
    struct Athena_SoundContext *const ctx = calloc(sizeof(struct Athena_SoundContext *));
    DirectSoundCreate(0, &(ctx->direct_sound), 0);
}

void Athena_DestroySoundContext(struct Athena_SoundContext *ctx){
    ctx->direct_sound->Release();
    ctx->direct_sound = NULL;
}

struct Athena_Sound *Athena_CreateSound(struct Athena_SoundContext *ctx){
    struct Athena_Sound *snd = calloc(sizeof(struct Athena_Sound *));
    snd->ctx = ctx;
    return snd;
}

void Athena_DestroySound(struct Athena_Sound *snd){
    if(snd->buffer)
        snd->buffer->Release();
}

void Athena_SoundInit(struct Athena_Sound *, unsigned num_channels, unsigned samples_per_second, enum Athena_SoundFormat format){
    snd->wfx->wFormatTag = WAVE_FORMAT_PCM;
    if(num_channels == 2)
        snd->wfx->nChannels = 2;
    else
        snd->wfx->nChannels = 1;
    
    snd->wfx->nSamplesPerSecond = samples_per_second;

    if(format==Athena_SoundU16)
        snd->wfx->wBitsPerSample = 16;
    else if(format==Athena_SoundU32)
        snd->wfx->wBitsPerSample = 32;
    else
        snd->wfx->wBitsPerSample = 8;

}

void Athena_SoundGetConfig(const struct Athena_Sound *sound, struct Athena_SoundConfig *to);
void Athena_SoundSetConfig(struct Athena_Sound *sound, const struct Athena_SoundConfig *to);

float Athena_SoundGetLength(const struct Athena_Sound *sound);
unsigned Athena_SoundGetChannels(const struct Athena_Sound *sound);
unsigned Athena_SoundGetSamplesPerSecond(const struct Athena_Sound *sound);
enum Athena_SoundFormat Athena_SoundGetFormat(const struct Athena_Sound *sound);

unsigned Athena_SoundPost(struct Athena_Sound *sound, const void *data, unsigned length);

void Athena_SoundPlay(struct Athena_Sound *sound);
void Athena_SoundPause(struct Athena_Sound *sound);
void Athena_SoundStop(struct Athena_Sound *sound);
void Athena_SoundRewind(struct Athena_Sound *sound);
