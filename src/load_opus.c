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

/*
int Athena_LoadOpusSoundMemory(const void *data, int size, struct Athena_Sound *to, struct Athena_SoundContext *ctx){
    int err = 0;

    int16_t buffer[ATHENA_BUFFER_SIZE];

    OpusDecoder *const decoder = opus_decoder_create(48000, 2, &err);
    if(err!= OPUS_OK)
        return err;

    Athena_SoundInit(to, 2, 48000, Athena_SoundU16);

    while((err = opus_decode(decoder, data, size, buffer, ATHENA_BUFFER_SIZE >> 1, 0))){
        data = ((int16_t *)data) + err;
        Athena_SoundPost(to, buffer, sizeof buffer);
    }

    return 0;
}
*/

#define ATHENA_BUFFER_SIZE ( 0x100 << 8 )

static int athena_load_opus_inner(const void *data, int size, struct Athena_Sound *to, struct Athena_SoundContext *ctx, ogg_sync_state *state, ogg_page *page, ogg_stream_state *stream, OpusDecoder *decoder){
    if(ogg_sync_pageout(state, page) == 1){
        ogg_packet packet;
        if(ogg_page_bos(page)){
            ogg_stream_init(stream, ogg_page_serialno(page));
        }

        ogg_stream_pagein(stream, page);
        if(ogg_stream_packetout(stream, &packet)==1){
            
            /* 5760? */
            const unsigned buffer_size = 10000 * sizeof(int16_t) * 2; /* num_channels */
            int16_t *buffer = malloc(buffer_size);
            
            int r = opus_decode(decoder, packet.packet, packet.bytes, buffer, 10000, 0);

            if(r>=0)
                Athena_SoundPost(to, buffer, r * sizeof(int16_t));
            else{
                fputs("[athena_load_opus_inner]", stderr);
                switch(r){
                    case OPUS_INVALID_PACKET:
                        fputs("Invalid Opus packet", stderr);
                        break;
                    case OPUS_BUFFER_TOO_SMALL:
                        fputs("Opus buffer too small", stderr);
                        break;
                    case OPUS_INTERNAL_ERROR:
                        fputs("Opus internal error", stderr);
                        break;
                    case OPUS_INVALID_STATE:
                        fputs("Invalid Opus state (not initialized or double freed)", stderr);
                        break;
                    case OPUS_ALLOC_FAIL:
                        fputs("OOM", stderr);
                        break;
                }
                fputc('\n', stderr);
            }
            
            free(buffer);
        }
    }
    
    if(size <= 0)
        return 0;
    else{
        const unsigned long buffer_size = (size<ATHENA_BUFFER_SIZE)?size:ATHENA_BUFFER_SIZE;
        char *buffer = ogg_sync_buffer(state, buffer_size);
        memcpy(buffer, data, buffer_size);
        data = (uint8_t *)data + buffer_size;
        size -= buffer_size;
        
        ogg_sync_wrote(state, buffer_size);
        
        return athena_load_opus_inner(data, size, to, ctx, state, page, stream, decoder);
    }
}

int Athena_LoadOpusSoundMemory(const void *data, int size, struct Athena_Sound *to, struct Athena_SoundContext *ctx){
    int err = 0;

    OpusDecoder *const decoder = opus_decoder_create(48000, 2, &err);
    ogg_sync_state state;
    ogg_page page;
    ogg_stream_state stream;
    
    if(err!=OPUS_OK)
        return -1;
    
    err = ogg_sync_init(&state);
    
    Athena_SoundInit(to, 2, 48000, Athena_SoundU16);

    athena_load_opus_inner(data, size, to, ctx, &state, &page, &stream, decoder);

    opus_decoder_destroy(decoder);

    return err;
}
