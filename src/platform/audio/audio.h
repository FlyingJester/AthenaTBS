#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Athena_SoundContext;
struct Athena_Sound;

struct Athena_SoundConfig{
    float volume, pan;
    unsigned char loop;
};

enum Athena_SoundFormat { Athena_SoundU8, Athena_SoundS16, Athena_SoundS24, Athena_SoundS32, Athena_SoundFloat };
#define ATHENA_FORMAT_BYTE_SIZE(FORMAT_)\
    (FORMAT_ == Athena_SoundFloat) ? 4 :\
    (FORMAT_ + 1)

#define ATHENA_VOLUME_EPSILON 0.0025f

/* This MUST be either 16-bit integer or 32-bit float. */
enum Athena_SoundFormat Athena_PreferredFormat(struct Athena_SoundContext *);
enum Athena_SoundFormat Athena_PreferredSoundFormat(struct Athena_Sound *);
unsigned Athena_SupportsFormat(struct Athena_SoundContext *, enum Athena_SoundFormat);

struct Athena_SoundContext *Athena_CreateSoundContext();
void Athena_DestroySoundContext(struct Athena_SoundContext *);

struct Athena_Sound *Athena_CreateSound(struct Athena_SoundContext *);
void Athena_DestroySound(struct Athena_Sound *);

void Athena_SoundInit(struct Athena_Sound *, unsigned num_channels, unsigned samples_per_second, enum Athena_SoundFormat format);

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

#ifdef __cplusplus
}
#endif
