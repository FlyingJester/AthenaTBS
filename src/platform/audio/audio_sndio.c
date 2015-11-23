#include "audio.h"
#include <sndio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

/* The sndio backend has no actual Athena_SoundContext structure. All information is stored
 * per-sound. */

struct Athena_Sound{
	struct sio_hdl *handle;
	struct sio_par par;
	int written;
	/* Since most of the config members are NOT in sio_par, we just have an entire config object here. */
	struct Athena_SoundConfig config;
};

enum Athena_SoundFormat Athena_PreferredFormat(struct Athena_SoundContext *ctx){
	return Athena_SoundU16;
}

enum Athena_SoundFormat Athena_PreferredSoundFormat(struct Athena_Sound *snd){
	assert(snd);
	return Athena_SoundU16;
}

/* sndio can handle between 8 and 64 bit (maybe more?) signed or unsigned audio. */
unsigned Athena_SupportsFormat(struct Athena_SoundContext *ctx, enum Athena_SoundFormat format){
	return format!=Athena_SoundFloat;
}

struct Athena_SoundContext *Athena_CreateSoundContext(){
	return (void *)1;
}

void Athena_DestroySoundContext(struct Athena_SoundContext *ctx){
	/* Contexts are dummy objects in the sndio backend */
}

struct Athena_Sound *Athena_CreateSound(struct Athena_SoundContext *ctx){
	struct Athena_Sound *snd = calloc(sizeof(struct Athena_Sound), 1);
	snd->handle = sio_open(SIO_DEVANY, SIO_PLAY, 1);
	return snd;
}

void Athena_DestroySound(struct Athena_Sound *snd){
	if(snd->handle){
		sio_close(snd->handle);
	}
	free(snd);
}

void Athena_SoundInit(struct Athena_Sound *snd, unsigned num_c, unsigned rate, enum Athena_SoundFormat format){
	/* We aren't going to touch this. */
	if(format==Athena_SoundFloat)
		return;
	if(!snd->handle)
		return;

	sio_getpar(snd->handle, &snd->par);
	switch(format){
		case Athena_SoundU16:
			snd->par.bits = 16;
			snd->par.bps = 2;
			break;
		case Athena_SoundU32:
			snd->par.bits = 32;
			snd->par.bps = 4;
			break;
		default:
			snd->par.bits = 8;
			snd->par.bps = 1;
	}
	/* We really need to fix this in every other backend. But for now, we just act broken here */
	snd->par.sig = 0;

	snd->par.rchan = 0;
	snd->par.pchan = num_c;

	snd->par.rate = rate;

	snd->par.xrun = SIO_IGNORE;

	sio_setpar(snd->handle, &snd->par);
	sio_setvol(snd->handle, SIO_MAXVOL);

	snd->config.volume = 1.0;
	snd->config.pan = 0.0f;
	snd->config.loop = 0;
}

void Athena_SoundGetConfig(const struct Athena_Sound *snd, struct Athena_SoundConfig *to){
	to[0] = snd->config;
}

void Athena_SoundSetConfig(struct Athena_Sound *snd, const struct Athena_SoundConfig *from){
	if(abs(snd->config.volume - from->volume) > 0.01f){
		sio_setvol(snd->handle, from->volume * SIO_MAXVOL);
	}

	snd->config = *from;
}

float Athena_SoundGetLength(const struct Athena_Sound *snd){
	if(!snd->par.bps)
		return 0;
	return ((float)snd->written) / ((float)(snd->par.bps * snd->par.rate));
}

unsigned Athena_SoundGetChannels(const struct Athena_Sound *snd){
	return snd->par.pchan;
}

unsigned Athena_SoundGetSamplesPerSecond(const struct Athena_Sound *snd){
	return snd->par.rate;
}

enum Athena_SoundFormat Athena_SoundGetFormat(const struct Athena_Sound *snd){
	switch(snd->par.bps){
		case 1:
		/* Lolwut? */
		case 2:
			return Athena_SoundU16;
		case 4:
			return Athena_SoundU32;
		default:
		return 0;
	}
}

unsigned Athena_SoundPost(struct Athena_Sound *snd, const void *data, unsigned length){
	if(snd->handle){
		snd->written += length;
		sio_write(snd->handle, data, length);
		return length;
	}
	else
		return 0;
}

void Athena_SoundPlay(struct Athena_Sound *snd){
	sio_start(snd->handle);
}

void Athena_SoundPause(struct Athena_Sound *snd){
	sio_stop(snd->handle);
}

void Athena_SoundStop(struct Athena_Sound *snd){
	sio_stop(snd->handle);
}

void Athena_SoundRewind(struct Athena_Sound *snd){

}
