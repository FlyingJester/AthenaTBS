#pragma once
#include "viewport.h"
#include <stdint.h>

struct Athena_Viewport;

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
void Athena_BlitBlended(const struct Athena_Image *src, struct Athena_Image *dst, int x, int y);
void Athena_BlitBlendMode(const struct Athena_Image *src, struct Athena_Image *dst, int x, int y, 
    uint32_t (*blend_func)(uint32_t src, uint32_t dst));

void Athena_CloneImage(struct Athena_Image *to, const struct Athena_Image *from);

void Athena_SetPixel(struct Athena_Image *to, int x, int y, uint32_t color);
uint32_t Athena_GetPixel(struct Athena_Image *to, int x, int y);
void Athena_FillRect(struct Athena_Image *to, int x, int y, unsigned w, unsigned h, uint32_t color);
void Athena_FillViewport(struct Athena_Viewport *v, uint32_t color);

void Athena_CreateImage(struct Athena_Image *that, unsigned w, unsigned h);
void Athena_DestroyImage(struct Athena_Image *that);

/*
 * Semi-private function for blitting a certain line of src. 
 * Only performs horizontal clipping. 
 */
void Athena_BlitScanLine(const struct Athena_Image *src, struct Athena_Image *dst, unsigned line, unsigned x, unsigned y);
void Athena_BlitScanLineBlended(const struct Athena_Image *src, struct Athena_Image *dst, unsigned line, unsigned x, unsigned y);

/*
 * Semi-private, does no clipping.
 */
uint32_t *Athena_Pixel(struct Athena_Image *to, int x, int y);
const uint32_t *Athena_PixelConst(const struct Athena_Image *to, int x, int y);

uint32_t Athena_RGBAToRaw(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void Athena_RawToRGBA(uint32_t rgba, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a);
uint8_t Athena_RawToR(uint32_t rgba);
uint8_t Athena_RawToG(uint32_t rgba);
uint8_t Athena_RawToB(uint32_t rgba);
uint8_t Athena_RawToA(uint32_t rgba);

uint32_t Athena_RGBARawBlend(uint32_t src, uint32_t dst);
uint32_t Athena_RGBABlend(uint8_t src_r, uint8_t src_g, uint8_t src_b, uint8_t src_a, uint8_t dst_r, uint8_t dst_g, uint8_t dst_b, uint8_t dst_a);

uint32_t Athena_RGBARawMultiply(uint32_t src, uint32_t dst);
uint32_t Athena_RGBAMultiply(uint8_t src_r, uint8_t src_g, uint8_t src_b, uint8_t src_a, uint8_t dst_r, uint8_t dst_g, uint8_t dst_b, uint8_t dst_a);

uint32_t Athena_RGBARawAverage(uint32_t src, uint32_t dst);
uint32_t Athena_RGBAAverage(uint8_t src_r, uint8_t src_g, uint8_t src_b, uint8_t src_a, uint8_t dst_r, uint8_t dst_g, uint8_t dst_b, uint8_t dst_a);

uint32_t Athena_RGBARawReplace(uint32_t src, uint32_t dst);
uint32_t Athena_RGBAReplace(uint8_t src_r, uint8_t src_g, uint8_t src_b, uint8_t src_a, uint8_t dst_r, uint8_t dst_g, uint8_t dst_b, uint8_t dst_a);

uint32_t Athena_RGBARawAdd(uint32_t src, uint32_t dst);
uint32_t Athena_RGBAAdd(uint8_t src_r, uint8_t src_g, uint8_t src_b, uint8_t src_a, uint8_t dst_r, uint8_t dst_g, uint8_t dst_b, uint8_t dst_a);

void Athena_BlendRect(struct Athena_Image *to, int x, int y, unsigned w, unsigned h, uint32_t color, uint32_t (*blend_func)(uint32_t src, uint32_t dst));
void Athena_BlendViewport(struct Athena_Viewport *v, uint32_t color, uint32_t (*blend_func)(uint32_t src, uint32_t dst));
void Athena_BlendPixel(struct Athena_Image *to, int x, int y, uint32_t color, uint32_t (*blend_func)(uint32_t src, uint32_t dst));

void Athena_MaskImage(struct Athena_Image *image, uint32_t color);
/* From and to can be the same image safely. */
void Athena_FlipImageVertically(const struct Athena_Image *from, struct Athena_Image *to);

/* It's up to the caller to know that the palette is big enough and that data can cover the entire image. */
void Athena_ImageFromPalette(struct Athena_Image *to, const uint8_t *data, const uint32_t *palette);

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
