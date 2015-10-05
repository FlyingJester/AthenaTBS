#pragma once
#include <stdint.h>

struct Athena_Image{
    uint32_t *pixels;
    unsigned w, h;
};

struct Athena_ImageArray{
    struct Athena_Image *images;
    unsigned num_images, images_capacity;
};

void Athena_CreateImageArray(struct Athena_ImageArray *ia);
void Athena_DestroyImageArray(struct Athena_ImageArray *ia);

void Athena_Blit(const struct Athena_Image *src, struct Athena_Image *dst, int x, int y);
void Athena_CloneImage(struct Athena_Image *to, const struct Athena_Image *from);

void Athena_CreateImage(struct Athena_Image *that, unsigned w, unsigned h);
void Athena_DestroyImage(struct Athena_Image *that);

/*
 * Semi-private function for blitting a certain line of src. 
 * Only performs horizontal clipping. 
 */
void Athena_BlitScanLine(const struct Athena_Image *src, struct Athena_Image *dst, unsigned line, unsigned x, unsigned y);

#define ATHENA_LOADPNG_SUCCESS 0u
#define ATHENA_LOADPNG_NO_FILE 1u
#define ATHENA_LOADPNG_BADFILE 2u
#define ATHENA_LOADPNG_NFORMAT 4u
#define ATHENA_LOADPNG_IS_NULL 8u
#define ATHENA_LOADPNG_PNG_ERR 16u

/*
 * Returns 0 on success
 */
unsigned Athena_LoadAuto(struct Athena_Image *to, const char *path);
unsigned Athena_LoadPNG(struct Athena_Image *to, const char *path);
unsigned Athena_LoadTGA(struct Athena_Image *to, const char *path);
