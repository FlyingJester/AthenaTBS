#include "font.h"
#include "cynical.h"
#include "sgi_screen.h"
#include "selawik.h"
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
        font->glyphs = calloc(num_chars, sizeof(struct Athena_Image));
        /* Real data starts at 64. */
            
        while(i<num_chars){
            unsigned w = ((const uint16_t *)(mem + offset))[0],
                h = ((const uint16_t *)(mem + offset))[1];

            offset+=8;

            if(size<(offset+(w*h))*4){
                puts("Unexpected End of File");

                while(i--) /* This is fine since font->glyphs[i] is not yet valid. */
                    Athena_DestroyImage(font->glyphs + i);

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

struct Athena_Font *LoadStaticFontMem(void *mem_z, const unsigned long size){
    uint32_t *mem = mem_z;
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
        font->glyphs = calloc(num_chars, sizeof(struct Athena_Image));
        /* Real data starts at 64. */
            
        while(i<num_chars){
            unsigned w = ((const uint16_t *)(mem + offset))[0],
                h = ((const uint16_t *)(mem + offset))[1];

            offset+=8;

            if(size<(offset+(w*h))*4){
                puts("Unexpected End of File");
                free(font->glyphs);
                return NULL;
            }

            font->glyphs[i].w = w;
            font->glyphs[i].h = h;
            font->glyphs[i].pixels = mem + offset;
            offset+=w*h;
            i++;
        }
    }
    return font;
}

void DestroyStaticFont(struct Athena_Font *font){
    free(font->glyphs);
    free(font);
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

        Athena_BlitBlended(surf, onto, x, y);

        x+=surf->w;
        
        str++;
    }
}
void WriteStringN(struct Athena_Font *font, const char *str, unsigned long n, struct Athena_Image *onto, unsigned x, unsigned y, uint32_t color){
    if(!str)
        return;
    while(*str!='\0' && n--){
        struct Athena_Image* const surf = GetBoundedGlyph(font, *str);

        Athena_BlitMasked(surf, onto, x, y, color);

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

static struct Athena_Font *title_font = NULL,
    * system_font = NULL,
    * mono_font = NULL;

#if 0

struct Athena_Font *GetSystemFont(){
    if(system_font==NULL){
#if ( defined ATHENA_USE_CYNICAL ) && ( !defined ATHENA_USE_SELAWIK )
        system_font = LoadFontMem((uint32_t *)cynical_rfn, CYNICAL_RFN_SIZE);
#else
        system_font = LoadFont("res/fonts/selawik/selawik-small-bold.rfn");
#endif

/*
#ifdef EMBED_CYNICAL

#else
        system_font = LoadFont("res/fonts/cynical.rfn");
#endif
        system_font = LoadFont("res/fonts/sgi/sgi_screen.rfn");
*/
    }

    return system_font;
}

#endif

struct Athena_Font *GetSystemFont(){
    if(system_font==NULL)
        system_font = LoadStaticFontMem((uint32_t *)sgi_screen_rfn, SGI_SCREEN_RFN_SIZE);
    return system_font;
}

struct Athena_Font *GetMonoFont(){
    if(mono_font==NULL)
        mono_font = LoadStaticFontMem((uint32_t *)sgi_screen_rfn, SGI_SCREEN_RFN_SIZE);
    return mono_font;
}

struct Athena_Font *GetTitleFont(){
    if(title_font==NULL){
        title_font = LoadStaticFontMem((uint32_t *)selawik_small_bold_rfn, SELAWIK_SMALL_BOLD_RFN_SIZE);
/*        PolarizeFont(title_font); */
    }
    return title_font;
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
    if(system_font)
        DestroyStaticFont(system_font);
}

void DestroyTitleFont(){
    if(title_font)
        DestroyStaticFont(title_font);
}

void DestroyMonoFont(){
    if(mono_font)
        DestroyStaticFont(mono_font);
}

void PolarizeFont(struct Athena_Font *font){
/*
    unsigned long number_glyphs;
    struct Athena_Image *glyphs;
*/
    int i;
    for(i=0; i<font->number_glyphs; i++){
        int z = font->glyphs[i].w * font->glyphs[i].h;
        uint8_t *a = (uint8_t *)(font->glyphs[i].pixels);
        a += 3;
        while(--z){
/*
            if(a[0]<32)
                a[0] = 0;
            else if(a[0]<96)
                a[0] = 64;
*/
            if(a[0]<96)
                a[0] = 0;
            else if(a[0]<160)
                a[0] = 128;
            else
                a[0] = 0xFF;
            a+=4;
        }
    }
}
