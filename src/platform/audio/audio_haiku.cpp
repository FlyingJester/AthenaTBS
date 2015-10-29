#include "audio.h"

#include <media/SoundPlayer.h>
#include <game/SimpleGameSound.h>
#include <game/GameSoundDefs.h>
#include <vector>
#include <assert.h>

struct Athena_SoundContext;
struct Athena_Sound {
	Athena_SoundFormat format;
	unsigned samples_per_second, num_channels;

	std::vector<float> samples;
	BGameSound *sound;
};

enum Athena_SoundFormat Athena_PreferredFormat(struct Athena_SoundContext *x){
	return Athena_SoundFloat;
}

enum Athena_SoundFormat Athena_PreferredSoundFormat(struct Athena_Sound *x){
	return Athena_PreferredFormat(NULL);
}

unsigned Athena_SupportsFormat(struct Athena_SoundContext *ctx, enum Athena_SoundFormat format){
	return format==Athena_SoundU16 || format==Athena_SoundU32 || format==Athena_SoundFloat;
}

Athena_SoundContext *Athena_CreateSoundContext(){
	return NULL;
}

void Athena_DestroySoundContext(struct Athena_SoundContext *ctx){
	assert(!ctx);
}

Athena_Sound *Athena_CreateSound(struct Athena_SoundContext *ctx){
	assert(!ctx);
	Athena_Sound *snd = new Athena_Sound();
	return snd;
}

void Athena_DestroySound(struct Athena_Sound *snd){
	delete snd->sound;
	delete snd;
}

void Athena_SoundInit(struct Athena_Sound *sound, unsigned num_channels, unsigned samples_per_second, enum Athena_SoundFormat format){
	sound->num_channels = num_channels;
	sound->samples_per_second = samples_per_second;
	sound->format = format;
}
/*
    float volume, pan;
    unsigned char loop;
*/
void Athena_SoundGetConfig(const struct Athena_Sound *sound, struct Athena_SoundConfig *to){
	if(!sound->sound)
		return;

	gs_attribute attr = { B_GS_LOOPING };
	sound->sound->GetAttributes(&attr, 1);

	to->loop = attr.value;
	to->volume = sound->sound->Gain();
	to->pan = sound->sound->Pan();
}

void Athena_SoundSetConfig(struct Athena_Sound *sound, const struct Athena_SoundConfig *to){
	if(!sound->sound)
		return;

	gs_attribute attr = { B_GS_LOOPING, 0, static_cast<float>(to->loop) };
	sound->sound->SetAttributes(&attr, 1);

	sound->sound->SetGain(to->volume);
	sound->sound->SetPan(to->pan);
}

float Athena_SoundGetLength(const struct Athena_Sound *sound){
	return static_cast<float>(sound->samples.size()) / static_cast<float>(sound->samples_per_second) / sound->num_channels;
}

unsigned Athena_SoundGetChannels(const struct Athena_Sound *sound){
	return sound->num_channels;
}

unsigned Athena_SoundGetSamplesPerSecond(const struct Athena_Sound *sound){
	return sound->samples_per_second;
}

enum Athena_SoundFormat Athena_SoundGetFormat(const struct Athena_Sound *sound){
	return sound->format;
}

unsigned Athena_SoundPost(struct Athena_Sound *sound, const void *z_data, unsigned length){
	if(sound->sound)
		return 1;
	const float *data = static_cast<const float *>(z_data);
	sound->samples.insert(sound->samples.end(), data, data + (length / sizeof(float)));
	return 0;
}

void Athena_SoundPlay(struct Athena_Sound *sound){
	if(!sound->sound){
		gs_audio_format f = { static_cast<float>(sound->samples_per_second) / 1000.0f, 0, 0, sound->samples.size() };
		if(sound->format == Athena_SoundFloat)
			f.format = 0x24;
		else if(sound->format == Athena_SoundU16)
			f.format = 0x02;
		else if(sound->format == Athena_SoundU32)
			f.format = 0x04;
		
		sound->sound = new BSimpleGameSound(sound->samples.data(), sound->samples.size(), &f, NULL);
	}
	sound->sound->StartPlaying();
}

void Athena_SoundPause(struct Athena_Sound *sound){
	if(!sound->sound)
		return;
	sound->sound->StopPlaying();
}

void Athena_SoundStop(struct Athena_Sound *sound){
	if(!sound->sound)
		return;
	sound->sound->StopPlaying();
}

void Athena_SoundRewind(struct Athena_Sound *sound){
	// Calling start on a sound restarts it in Be/Haiku.
	Athena_SoundPlay(sound);
}
