#include "font.h"
#include "bufferfile/bufferfile.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Athena_Font *LoadFont(const char *filename){
    int size;
    const uint32_t *mem = BufferFile(filename, &size);
    if(mem && size){

        struct Athena_Font *font = LoadFontMem(mem, size);

        FreeBufferFile((void *)mem, size);
        return font;

    }
    return NULL;
}

struct Athena_Font *LoadFontMem(const void *mem_z, const unsigned long size){
    const uint32_t *mem = mem_z;
    struct Athena_Font *font = malloc(sizeof(struct Athena_Font));
        
#ifdef __EMSCRIPTEN__
    mem = (uint32_t *)(((uint8_t*)(mem))-1);
#endif

const uint32_t sig = 
#ifdef __EMSCRIPTEN__
    0x66722e82;
#else
    0x6e66722e;
#endif


    if(mem[0]!=sig){ /* 0x66722e82 */
        printf("Bad Signature %x\n", mem[0]);
        return NULL;
    }
    if(((const uint16_t *)(&mem[1]))[0] != 2)
        return NULL;
    else{
        unsigned i = 0, offset = 64;
        const unsigned num_chars = ((const uint16_t *)(&mem[1]))[1];
        font->number_glyphs = num_chars;
        font->glyphs = malloc(num_chars<<3);
        /* Real data starts at 64. */
            
        while(i<num_chars){
            unsigned w = ((const uint16_t *)(mem + offset))[0],
                h = ((const uint16_t *)(mem + offset))[1];

            offset+=8;

            if(size<(offset+(w*h))*4){
                puts("Unexpected End of File");
                /* TODO: HUGE MEMORY LEAK! */
                free(font->glyphs);
                return NULL;
            }

            Athena_CreateImage(font->glyphs + i, w, h);
            memcpy(font->glyphs[i].pixels, mem + offset, w*h<<2);
            offset+=w*h;
            i++;
        }
    }
    return font;
}

struct Athena_Image *GetBoundedGlyph(struct Athena_Font *font, unsigned i){
    if(i<font->number_glyphs)
        return font->glyphs + i;
    else if('x'>font->number_glyphs)
        return font->glyphs + 'x';
    else
        return font->glyphs;
}

void WriteString(struct Athena_Font *font, const char *str, struct Athena_Image *onto, unsigned x, unsigned y){
    if(!str)
        return;
    while(*str!='\0'){
        struct Athena_Image* const surf = GetBoundedGlyph(font, *str);

        Athena_Blit(surf, onto, x, y);

        x+=surf->w;
        
        str++;
    }
}
void WriteStringN(struct Athena_Font *font, const char *str, unsigned long n, struct Athena_Image *onto, unsigned x, unsigned y){
    if(!str)
        return;
    while(*str!='\0' && n--){
        struct Athena_Image* const surf = GetBoundedGlyph(font, *str);

        Athena_Blit(surf, onto, x, y);

        x+=surf->w;
        
        str++;
    }
}
unsigned long StringWidth(struct Athena_Font *font, const char *str){ return StringWidthN(font, str, ~0u); }
unsigned long StringWidthN(struct Athena_Font *font, const char *str, unsigned long n){
    unsigned long w = 0;
    while(*str!='\0' && n){
        w+=GetBoundedGlyph(font, *str)->w;
        str++;
        n--;
    }
    return w;
}

#ifdef EMBED_CYNICAL
#include "cynical.inc"
#endif

static struct Athena_Font *system_font = NULL;

struct Athena_Font *GetSystemFont(){
    if(system_font==NULL)
#ifdef EMBED_CYNICAL
        system_font = LoadFontMem((uint32_t *)res_fonts_cynical_rfn, res_fonts_cynical_rfn_len);
#else
        system_font = LoadFont("res/fonts/cynical.rfn");
#endif
    return system_font;
}

void DestroyFont(struct Athena_Font *font){
    uint64_t i = 0;
    
    if(!font) return;
    while(i<font->number_glyphs){
        Athena_DestroyImage(font->glyphs + i);
        i++;
    }
    
    free(font->glyphs);
    free(font);
}

void DestroySystemFont(){
    DestroyFont(system_font);
}
