#pragma once
#include "image.h"

#ifdef __cplusplus
extern "C" {
#endif
    struct Athena_Font {
        unsigned long number_glyphs;
        struct Athena_Image *glyphs;
    };
    
    struct Athena_Font *LoadFont(const char *filename);
    struct Athena_Font *LoadFontMem(const void *mem_z, const unsigned long size_in_bytes);
    void DestroyFont(struct Athena_Font *font);
    void WriteString(struct Athena_Font *font, const char *str, struct Athena_Image *onto, unsigned x, unsigned y);
    void WriteStringN(struct Athena_Font *font, const char *str, unsigned long n, struct Athena_Image *onto, unsigned x, unsigned y, uint32_t color);
    
    unsigned long StringWidth(struct Athena_Font *font, const char *str);
    unsigned long StringWidthN(struct Athena_Font *font, const char *str, unsigned long n);

    struct Athena_Font *GetTitleFont();
    struct Athena_Font *GetSystemFont();
    struct Athena_Font *GetMonoFont();
    
    /* This will clean up poor conversions of TTF fonts.
     * Any pixel with less than 96 alpha will be made completely transparent.
     * Any pixel with between 96 and 160 alpha will be made half transparent.
     * Any pixel with more than 160 alpha will be made completely opaque.
     */
    void PolarizeFont(struct Athena_Font *font);

    /* This can be called to free up a tiny bit more memory (about a megabyte),
     * or just to invalidate the reference and reload the system font. */
    void DestroyTitleFont();
    void DestroySystemFont();
    void DestroyMonoFont();

    struct Athena_Image *GetBoundedGlyph(struct Athena_Font *font, unsigned i);

#ifdef __cplusplus
}
#endif
