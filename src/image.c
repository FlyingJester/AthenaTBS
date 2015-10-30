#include "image.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include "memset_pattern4.h"

#define ATHENA_MIN(A, B) (((A)>(B))?(B):(A))

void Athena_CreateImageArray(struct Athena_ImageArray *ia){
    ia->images = malloc(sizeof(struct Athena_Image) * 8);
    ia->num_images = 0;
    ia->images_capacity = 8;
}

static void athena_destroy_image_array(struct Athena_Image *images, unsigned length){
    if(!length)
        return;
    else{
        Athena_DestroyImage(images);
        athena_destroy_image_array(images + 1, length - 1);
    }
}

void Athena_DestroyImageArray(struct Athena_ImageArray *ia){
    athena_destroy_image_array(ia->images, ia->num_images);

    free(ia->images);
    ia->num_images = 0;
    ia->images_capacity = 0;
}

void Athena_DestroyImage(struct Athena_Image *that){
    free(that->pixels);
    that->pixels = NULL;
    that->w = that->h = 0;
}

static unsigned Athena_LowerBlitWidth(const struct Athena_Image *src, const struct Athena_Image *dst, unsigned x){
    const unsigned clip = dst->w - x;
    if(src->w<clip)
        return src->w;
    else
        return clip;
}

void Athena_LowerBlitScanLine(const struct Athena_Image *src, struct Athena_Image *dst, unsigned line, unsigned len, unsigned x, unsigned y){
    memcpy(dst->pixels + x + (y * dst->w), src->pixels + (line * src->w), len << 2);
}

/* #define ATHENA_DISABLE_OPT_BLEND 
    #pragma error
*/

#ifndef ATHENA_DISABLE_OPT_BLEND

static unsigned athena_find_solid_block(const struct Athena_Image *src, unsigned x, unsigned y){
/* the > 0xFA is a Slightly greedy fudge to improve performance. */
    if(x < src->w && Athena_RawToA( *Athena_PixelConst(src, x, y) ) > 0xFC)
        return athena_find_solid_block(src, x+1, y);

    return x;
}

static unsigned athena_find_empty_block(const struct Athena_Image *src, unsigned x, unsigned y){
/* the < 0x08 is a Slightly greedy fudge to improve performance. */
    if(x < src->w && Athena_RawToA( *Athena_PixelConst(src, x, y) ) < 0x04)
        return athena_find_empty_block(src, x+1, y);

    return x;
}

#endif

static int athena_blit_scanline_blended_iter(const struct Athena_Image *src, struct Athena_Viewport *to, unsigned laser_x, unsigned laser_y){
    if(laser_y >= to->h)
        return 0;
    else if(laser_x >= to->w)
        return athena_blit_scanline_blended_iter(src, to, 0, laser_y + 1);
    else{

        uint32_t *pixel_to = Athena_Pixel(to->image, to->x + laser_x, to->y + laser_y);
        const uint32_t *pixel_from = Athena_PixelConst(src, laser_x, laser_y);
#ifndef ATHENA_DISABLE_OPT_BLEND
        const unsigned empty_x = athena_find_empty_block(src, laser_x, laser_y),
            solid_x = athena_find_solid_block(src, laser_x, laser_y);
        if(solid_x > laser_x){
            const unsigned len = ATHENA_MIN(solid_x - laser_x, to->w - laser_x);
            memcpy(pixel_to, pixel_from, len << 2);
            return athena_blit_scanline_blended_iter(src, to, laser_x + len, laser_y);
        }
        else if(empty_x > laser_x){
            /* Just skip the zero alpha area */
            return athena_blit_scanline_blended_iter(src, to, empty_x, laser_y);
        }
        else
#endif
        {
            pixel_to[0] = Athena_RGBARawBlend(*pixel_from, *pixel_to);
            return athena_blit_scanline_blended_iter(src, to, laser_x + 1, laser_y);
        }
    }
}

void Athena_BlitScanLine(const struct Athena_Image *src, struct Athena_Image *dst, unsigned line, unsigned x, unsigned y){
    assert(src);
    assert(dst);
    Athena_LowerBlitScanLine(src, dst, line, Athena_LowerBlitWidth(src, dst, x), x, y);
}

static void Athena_LowerBlit(const struct Athena_Image *src, struct Athena_Image *dst, unsigned line, unsigned len, unsigned x, unsigned y){
    if((line < src->h) && (y < dst->h)){
        Athena_LowerBlitScanLine(src, dst, line, len, x, y);
        Athena_LowerBlit(src, dst, line+1, len, x, y+1);
    }
}

void Athena_Blit(const struct Athena_Image *src, struct Athena_Image *dst, int x, int y){
    assert(src);
    assert(dst);
    if(x < dst->w && y < dst->h && x + (long)src->w > 0 && y + (long)src->h > 0){
        const unsigned len = Athena_LowerBlitWidth(src, dst, x);
        if(len)
            Athena_LowerBlit(src, dst, 0, len, x, y);
    }
}

void Athena_BlitBlended(const struct Athena_Image *src, struct Athena_Image *dst, int x, int y){
    assert(src);
    assert(dst);
    if(x < dst->w && y < dst->h && x + (long)src->w > 0 && y + (long)src->h > 0){
        struct Athena_Viewport to;
        const unsigned len = Athena_LowerBlitWidth(src, dst, x);
        
        to.image = dst;
        to.x = x;
        to.y = y;
        to.w = len;
        to.h = ATHENA_MIN(src->h, dst->h - to.y);
        
        athena_blit_scanline_blended_iter(src, &to, 0, 0);
    }
}

/* This is not folded into athena_blit_scanline_blended_iter since the latter contains optimizations only valid for a few blend modes. */
static int athena_blit_scanline_blend_mode_iter(const struct Athena_Image *src, struct Athena_Viewport *to, unsigned laser_x, unsigned laser_y,
    uint32_t (*blend_func)(uint32_t src, uint32_t dst)){

    /* for the Blend blend mode, athena_blit_scanline_blended_iter is equivalent but more optimized. */
    if(blend_func==Athena_RGBARawBlend)
        return athena_blit_scanline_blended_iter(src, to, laser_x, laser_y);
    else if(laser_y >= to->h)
        return 0;
    else if(laser_x >= to->w)
        return athena_blit_scanline_blend_mode_iter(src, to, 0, laser_y + 1, blend_func);
    else{

        uint32_t * const pixel_to = Athena_Pixel(to->image, to->x + laser_x, to->y + laser_y);

        pixel_to[0] = blend_func(Athena_PixelConst(src, laser_x, laser_y)[0], *pixel_to);
        return athena_blit_scanline_blend_mode_iter(src, to, laser_x + 1, laser_y, blend_func);

    }
}


void Athena_BlitBlendMode(const struct Athena_Image *src, struct Athena_Image *dst, int x, int y, 
    uint32_t (*blend_func)(uint32_t src, uint32_t dst)){

    assert(src);
    assert(dst);

    if(blend_func==Athena_RGBARawReplace){
        Athena_Blit(src, dst, x, y);
    }
    else if(x < dst->w && y < dst->h && x + (long)src->w > 0 && y + (long)src->h > 0){
        struct Athena_Viewport to;
        const unsigned len = Athena_LowerBlitWidth(src, dst, x);
        
        to.image = dst;
        to.x = x;
        to.y = y;
        to.w = len;
        to.h = ATHENA_MIN(src->h, dst->h - to.y);
        
        athena_blit_scanline_blend_mode_iter(src, &to, 0, 0, (blend_func)?blend_func:Athena_RGBARawBlend);
    }
    
}

void Athena_CloneImage(struct Athena_Image *to, const struct Athena_Image *from){
    const unsigned pix_size = from->w * from->h << 2;
    to->w = from->w;
    to->h = from->h;   
    
    to->pixels = malloc(pix_size);
    memcpy(to->pixels, from->pixels, pix_size);
}

void Athena_CreateImage(struct Athena_Image *that, unsigned w, unsigned h){
    that->w = w;
    that->h = h;

    that->pixels = calloc(w<<1, h<<1);
}

void Athena_SetPixel(struct Athena_Image *to, int x, int y, uint32_t color){
    if(x < 0 || y < 0 || x >= to->w || y >= to->h)
        return;
    Athena_Pixel(to, x, y)[0] = color;
}

uint32_t Athena_GetPixel(struct Athena_Image *to, int x, int y){
    if(x < 0 || y < 0 || x >= to->w || y >= to->h)
        return 0;
    return *Athena_Pixel(to, x, y);
}

void Athena_FillViewport(struct Athena_Viewport *v, uint32_t color){
    Athena_FillRect(v->image, v->x, v->y, v->w, v->h, color);
}

void Athena_FillRect(struct Athena_Image *to, int x, int y, unsigned w, unsigned h, uint32_t color){
    if(!w || !h)
        return;
    if(x + (int)w < 0 || y + (int)h < 0 || x >= to->w || y >= to->h)
        return;
    
    if(x < 0 || y < 0){
        if(x < 0){
            w += x;
            x = 0;
        }
        if(y < 0){
            h += y;
            y = 0;
        }
        Athena_FillRect(to, x, y, w, h, color);
        return;
    }
    else if(w == 1){
        Athena_SetPixel(to, x, y, color);
        Athena_FillRect(to, x, y+1, 1, h-1, color); 
    }
    else{
        memset_pattern4(Athena_Pixel(to, x, y), &color, ATHENA_MIN(w, to->w - x)<<2);
        Athena_FillRect(to, x, y+1, w, h-1, color);
    }
}

static int athena_blend_rect_iter(struct Athena_Viewport *to, uint32_t color, unsigned laser_x, unsigned laser_y, uint32_t (*blend_func)(uint32_t src, uint32_t dst)){
    if(laser_y >= to->h)
        return 0;
    else if(laser_x >= to->w)
        return athena_blend_rect_iter(to, color, 0, laser_y + 1, blend_func);
    else{
        Athena_BlendPixel(to->image, to->x + laser_x, to->y + laser_y, color, blend_func);
        return athena_blend_rect_iter(to, color, laser_x + 1, laser_y, blend_func);
    }
}

void Athena_BlendRect(struct Athena_Image *dst, int x, int y, unsigned w, unsigned h, uint32_t color, uint32_t (*blend_func)(uint32_t src, uint32_t dst)){
    if(y >= dst->h || x>=dst->w)
        return;
    else if(w == 1 && h == 1)
        Athena_BlendPixel(dst, x, y, color, blend_func);
    else{
    
        struct Athena_Viewport to;
        to.image = dst;
        to.x = x;
        to.y = y;
        to.w = w;
        to.h = h;
        
        {
            int n_w = to.w, n_h = to.h;
            if(n_w + to.x > dst->w)
                n_w = dst->w - to.x;

            if(n_h + to.y > dst->h)
                n_h = dst->h - to.y;
        
            assert(n_w);
            assert(n_h);
            
            to.w = n_w;
            to.h = n_h;
        }
        
        Athena_BlendViewport(&to, color, blend_func);
    }
}

void Athena_BlendViewport(struct Athena_Viewport *v, uint32_t color, uint32_t (*blend_func)(uint32_t src, uint32_t dst)){
    athena_blend_rect_iter(v, color, 0, 0, blend_func);
}

void Athena_BlendPixel(struct Athena_Image *to, int x, int y, uint32_t color, uint32_t (*blend_func)(uint32_t src, uint32_t dst)){
    uint32_t *to_pixel = Athena_Pixel(to, x, y);
    to_pixel[0] = blend_func(*to_pixel, color);
}

void Athena_MaskImage(struct Athena_Image *image, uint32_t color){
    Athena_BlendRect(image, 0, 0, image->w, image->h, color, Athena_RGBARawMultiply);
}

uint32_t *Athena_Pixel(struct Athena_Image *to, int x, int y){
    return (uint32_t *)Athena_PixelConst(to, x, y);
}

const uint32_t *Athena_PixelConst(const struct Athena_Image *to, int x, int y){
    return to->pixels + x + (y * to->w);
}

/* 0xFF00FFFF is yellow. That is all. */

uint32_t Athena_RGBAToRaw(uint8_t r, uint8_t g, uint8_t b, uint8_t a){
    return (a << 24) | (b << 16) | (g << 8) | (r);
}

void Athena_RawToRGBA(uint32_t rgba, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a){
    r[0] = rgba & 0xFF;
    rgba >>= 8;
    g[0] = rgba & 0xFF;
    rgba >>= 8;
    b[0] = rgba & 0xFF;
    rgba >>= 8;
    a[0] = rgba & 0xFF;
}

uint8_t Athena_RawToR(uint32_t rgba){
    return rgba & 0xFF;
}

uint8_t Athena_RawToG(uint32_t rgba){
    return (rgba >> 8) & 0xFF;
}

uint8_t Athena_RawToB(uint32_t rgba){
    return (rgba >> 16) & 0xFF;
}

uint8_t Athena_RawToA(uint32_t rgba){
    return (rgba >> 24) & 0xFF;
}

#define ATHENA_DECONSTRUCT_BLENDER(NAME)\
uint32_t Athena_RGBARaw ## NAME(uint32_t src, uint32_t dst){\
    uint8_t src_r, src_g, src_b, src_a, dst_r, dst_g, dst_b, dst_a;\
    Athena_RawToRGBA(src, &src_r, &src_g, &src_b, &src_a);\
    Athena_RawToRGBA(dst, &dst_r, &dst_g, &dst_b, &dst_a);\
    return Athena_RGBA ## NAME(src_r, src_g, src_b, src_a, dst_r, dst_g, dst_b, dst_a);\
}

ATHENA_DECONSTRUCT_BLENDER(Blend)
ATHENA_DECONSTRUCT_BLENDER(Multiply)
ATHENA_DECONSTRUCT_BLENDER(Average)
ATHENA_DECONSTRUCT_BLENDER(Add)
ATHENA_DECONSTRUCT_BLENDER(Grayscale)

/* Slight shortcut. We don't need to disassemble the RGBA, since it's just a replacement. */
uint32_t Athena_RGBARawReplace(uint32_t src, uint32_t dst){
    return src;
}

#undef ATHENA_DECONSTRUCT_BLENDER

uint32_t Athena_RGBABlend(uint8_t src_r, uint8_t src_g, uint8_t src_b, uint8_t src_a, uint8_t dst_r, uint8_t dst_g, uint8_t dst_b, uint8_t dst_a){
    float accum_r = ((float)dst_r)/255.0f, accum_g = ((float)dst_g)/255.0f, accum_b = ((float)dst_b)/255.0f;
    
    const float src_factor = ((float)src_a)/255.0f, dst_factor = 1.0f - src_factor;
    
    accum_r = (accum_r * dst_factor) + ((((float)src_r)/255.0f) * src_factor);
    accum_g = (accum_g * dst_factor) + ((((float)src_g)/255.0f) * src_factor);
    accum_b = (accum_b * dst_factor) + ((((float)src_b)/255.0f) * src_factor);

    return Athena_RGBAToRaw(accum_r * 255.0f, accum_g * 255.0f, accum_b * 255.0f, 0xFF);
}

uint32_t Athena_RGBAMultiply(uint8_t src_r, uint8_t src_g, uint8_t src_b, uint8_t src_a, uint8_t dst_r, uint8_t dst_g, uint8_t dst_b, uint8_t dst_a){
    const uint16_t mul_r = src_r * dst_r, 
        mul_g = src_g * dst_g, 
        mul_b = src_b * dst_b, 
        mul_a = src_a * dst_a;

    return Athena_RGBAToRaw(mul_r / 0xFF, mul_g / 0xFF, mul_b / 0xFF, mul_a / 0xFF);
}

uint32_t Athena_RGBAAverage(uint8_t src_r, uint8_t src_g, uint8_t src_b, uint8_t src_a, uint8_t dst_r, uint8_t dst_g, uint8_t dst_b, uint8_t dst_a){
    const uint16_t ave_r = src_r + dst_r, 
        ave_g = src_g + dst_g, 
        ave_b = src_b + dst_b, 
        ave_a = src_a + dst_a;

    return Athena_RGBAToRaw(ave_r >> 1, ave_g >> 1, ave_b >> 1, ave_a >> 1);
}

uint32_t Athena_RGBAAdd(uint8_t src_r, uint8_t src_g, uint8_t src_b, uint8_t src_a, uint8_t dst_r, uint8_t dst_g, uint8_t dst_b, uint8_t dst_a){
    const uint16_t ave_r = src_r + dst_r, 
        ave_g = src_g + dst_g, 
        ave_b = src_b + dst_b, 
        ave_a = src_a + dst_a;

    return Athena_RGBAToRaw(
        (ave_r>0xFF)?0xFF:ave_r,
        (ave_g>0xFF)?0xFF:ave_g,
        (ave_b>0xFF)?0xFF:ave_b,
        (ave_a>0xFF)?0xFF:ave_a);
}

uint32_t Athena_RGBAReplace(uint8_t src_r, uint8_t src_g, uint8_t src_b, uint8_t src_a, uint8_t dst_r, uint8_t dst_g, uint8_t dst_b, uint8_t dst_a){
    return Athena_RGBAToRaw(src_r, src_g, src_b, src_a);
}

uint32_t Athena_RGBAGrayscale(uint8_t src_r, uint8_t src_g, uint8_t src_b, uint8_t src_a, uint8_t dst_r, uint8_t dst_g, uint8_t dst_b, uint8_t dst_a){
    const uint8_t new_level = (src_r + src_g + src_b) / 3;
    return Athena_RGBABlend(new_level, new_level, new_level, src_a, dst_r, dst_g, dst_b, dst_a);
}

static void athena_image_from_palette(uint32_t *to, const uint8_t *data, const uint32_t *palette, unsigned len){
    if(len){
        to[0] = palette[data[0]];
        athena_image_from_palette(to + 1, data + 1, palette, len - 1);
    }
}

void Athena_ImageFromPalette(struct Athena_Image *to, const uint8_t *data, const uint32_t *palette){
    athena_image_from_palette(to->pixels, data, palette, to->w * to->h);
}

void athena_flip_image_vertically_iter(const struct Athena_Image *from, struct Athena_Image *to, uint32_t *buffer, unsigned line){
    if(line > from->h >> 1){
        return;
    }

    memcpy(buffer, Athena_PixelConst(from, 0, line), from->w << 2);
    memcpy(Athena_Pixel(to, 0, line), Athena_PixelConst(from, 0, from->h - line - 1), from->w << 2);
    memcpy(Athena_Pixel(to, 0, from->h - line - 1), buffer, from->w << 2);
    athena_flip_image_vertically_iter(from, to, buffer, line + 1);
}

void Athena_FlipImageVertically(const struct Athena_Image *from, struct Athena_Image *to){
    if(from->w > to->w || from->h > to->h)
        return;
    else{
        uint32_t * const row_buffer = calloc(4, from->w);
        athena_flip_image_vertically_iter(from, to, row_buffer, 0);
        free(row_buffer);
    }
}

unsigned Athena_LoadAuto(struct Athena_Image *to, const char *path){
    const char * const end = path + strlen(path), *str = end;
    while(str[0] != '.'){
        if(str==path)
            return ATHENA_LOADPNG_NFORMAT;
        str--;
    }
    
    if(end - str == 4){
        if(memcmp(str, ".png", 4)==0)
            return Athena_LoadPNG(to, path);
        else if(memcmp(str, ".tga", 4)==0)
            return Athena_LoadTGA(to, path);
    }
    return ATHENA_LOADPNG_NFORMAT;
}
