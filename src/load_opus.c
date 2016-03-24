#include "load_opus.h"
#include "audio_ctl.h"
#include "bufferfile/bufferfile.h"
#include <opus/opus.h>
#include <ogg/ogg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Athena_Sound *Athena_LoadOpusFile(const char *path){
    return Athena_LoadOpusFileForContext(path, Athena_GetSystemSoundContext());
}


struct Athena_Sound *Athena_LoadOpusFileForContext(const char *path, struct Athena_SoundContext *ctx){
    int size;
    void * const data = BufferFile(path, &size);
    struct Athena_Sound *const sound = Athena_LoadOpusMemory(data, size, ctx);
    
    FreeBufferFile(data, size);
    
    return sound;
    
}


struct Athena_Sound *Athena_LoadOpusMemory(const void *data, int size, struct Athena_SoundContext *ctx){
    struct Athena_Sound * const sound = Athena_CreateSound(ctx);
    Athena_LoadOpusSoundMemory(data, size, sound, ctx);
    return sound;
}

#define ATHENA_BUFFER_SIZE ( 0x100 << 8 )

static int athena_push_stream(const void *data, int size, struct Athena_Sound *to, struct Athena_SoundContext *ctx, 
    ogg_sync_state *state, ogg_page *page, ogg_stream_state *stream, OpusDecoder *decoder, int *stream_inited){

    ogg_packet packet;
    if(!(*stream_inited))
        return 1;
    else if(ogg_stream_packetout(stream, &packet)==1){
        switch(Athena_PreferredSoundFormat(to)){
            case Athena_SoundS16:
            {
                /* 5760? */
                const unsigned buffer_size = 5760 * sizeof(int16_t) * 2; /* max num_channels */
                int16_t *buffer = malloc(buffer_size);
                int r = opus_decode(decoder, packet.packet, packet.bytes, buffer, buffer_size, 0);
                if(r>0)
                    Athena_SoundPost(to, buffer, r * sizeof(int16_t) * 2);
                free(buffer);
            }
            break;
            case Athena_SoundFloat:
            default:
            {
                const unsigned buffer_size = 5760 * sizeof(float) * 2; /* max num_channels */
                float *buffer = malloc(buffer_size);
                int r = opus_decode_float(decoder, packet.packet, packet.bytes, buffer, buffer_size, 0);
                if(r>0)
                    Athena_SoundPost(to, buffer, r * sizeof(float) * 2);
                free(buffer);
            }
            break;
        }

        return athena_push_stream(data, size, to, ctx, state, page, stream, decoder, stream_inited);
    }
    else
        return 0;
}

static int athena_push_page(const void *data, int size, struct Athena_Sound *to, struct Athena_SoundContext *ctx, ogg_sync_state *state, ogg_page *page, ogg_stream_state *stream, OpusDecoder *decoder, int *stream_inited){
    if(ogg_sync_pageout(state, page) == 1){
        if(ogg_page_bos(page)){
            ogg_stream_init(stream, ogg_page_serialno(page));
            stream_inited[0] = 1;
        }

        ogg_stream_pagein(stream, page);

        athena_push_stream(data, size, to, ctx, state, page, stream, decoder, stream_inited);
        return athena_push_page(data, size, to, ctx, state, page, stream, decoder, stream_inited);
    }
    else
        return 0;
}

static int athena_load_opus_inner(const void *data, int size, struct Athena_Sound *to, struct Athena_SoundContext *ctx, ogg_sync_state *state, ogg_page *page, ogg_stream_state *stream, OpusDecoder *decoder, int *stream_inited){
    athena_push_page(data, size, to, ctx, state, page, stream, decoder, stream_inited);
    
    if(size <= 0)
        return 0;
    else{
        const unsigned long buffer_size = (size<ATHENA_BUFFER_SIZE)?size:ATHENA_BUFFER_SIZE;
        char *buffer = ogg_sync_buffer(state, buffer_size);
        memcpy(buffer, data, buffer_size);
        data = (uint8_t *)data + buffer_size;
        size -= buffer_size;
        
        ogg_sync_wrote(state, buffer_size);
        
        return athena_load_opus_inner(data, size, to, ctx, state, page, stream, decoder, stream_inited);
    }
}

int Athena_LoadOpusSoundMemory(const void *data, int size, struct Athena_Sound *to, struct Athena_SoundContext *ctx){
    int err = 0;

    OpusDecoder *const decoder = opus_decoder_create(48000, 2, &err);
    ogg_sync_state state;
    ogg_page page;
    ogg_stream_state stream;
    
    int stream_inited = 0;
    
    if(err!=OPUS_OK)
        return -1;
    
    err = ogg_sync_init(&state);
    
    Athena_SoundInit(to, 2, 48000, 
        (Athena_PreferredSoundFormat(to)==Athena_SoundS16)?
        Athena_SoundS16:Athena_SoundFloat);

    athena_load_opus_inner(data, size, to, ctx, &state, &page, &stream, decoder, &stream_inited);

    opus_decoder_destroy(decoder);

    return err;
}
