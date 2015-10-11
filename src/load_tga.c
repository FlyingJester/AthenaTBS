#include "image.h"
#include "bufferfile/bufferfile.h"
#include <stdint.h>
#include <string.h>

#if (!defined(__APPLE__)) || ((defined(__FreeBSD__)) && (__FreeBSD__ < 9))

static void memset_pattern4(void *to, const unsigned char *pattern, unsigned long len){
    while(len--)
        to[len] = pattern[len % 4];
}

#endif

typedef uint32_t (*athena_tga_read_data)(const unsigned char **p_data, int *size);

#define ATHENA_TGA_COLOURMAP 1
#define ATHENA_TGA_RGBA 2
#define ATHENA_TGA_BW 3

struct Athena_TGAHeader{
    unsigned char id_length;
    unsigned char colourmap_type;
    unsigned short colourmap_origin;
    unsigned short colourmap_length;
    unsigned char colourmap_depth;
    
    short x_origin;
    short y_origin;
    short width;
    short height;

    unsigned char bits_per_pixel;
    char image_descriptor;

    /* Only RLE and raw is supported */
    unsigned char compression;
    unsigned char colour_type;
};

struct Athena_TGAImage{
    struct Athena_TGAHeader header;
    char id[0x100];

    struct Athena_Image image;
};

static unsigned short athena_lo_hi_bytes(const unsigned char *that){
    return ((unsigned short)that[0]) | (((unsigned short)that[1])<<8);
}

static void athena_tga_format_byte(struct Athena_TGAImage *image, unsigned char type){
    image->header.colour_type = type & 3;
    image->header.compression = (type & 8)!=0;
}

static void athena_tga_header_from_buffer(struct Athena_TGAImage *image, const unsigned char **p_data, int *size){
    const unsigned char *data = p_data[0];
    if(!data || !size || size[0] < 18)
        return;

    image->header.id_length = data[0];
    image->header.colourmap_type = data[1];
    
    athena_tga_format_byte(image, data[2]);

    image->header.colourmap_origin = athena_lo_hi_bytes(data + 3);
    image->header.colourmap_length = athena_lo_hi_bytes(data + 5);
    image->header.colourmap_depth = data[7];
    image->header.x_origin = athena_lo_hi_bytes(data + 8);
    image->header.y_origin = athena_lo_hi_bytes(data + 10);
    image->header.width = athena_lo_hi_bytes(data + 12);
    image->header.height = athena_lo_hi_bytes(data + 14);
    image->header.bits_per_pixel = data[16];
    image->header.image_descriptor = data[17];

    size[0] -= 18;
    data += 18;
}

static uint32_t athena_tga_read_rgba32(const unsigned char **p_data, int *size){
    const unsigned long color = (((unsigned long)athena_lo_hi_bytes(p_data[0] + 2))<<16) | athena_lo_hi_bytes(p_data[0]);
    p_data[0]+=4;
    size[0]--;
    return color;
}

static uint32_t athena_tga_read_rgba24(const unsigned char **p_data, int *size){
    const unsigned long color = (((unsigned long)athena_lo_hi_bytes(p_data[0] + 1))<<8) | p_data[0][0];
    p_data[0]+=3;
    size[0]--;
    return color;
}

static void athena_tga_read_raw(uint32_t **to, athena_tga_read_data reader, const unsigned char **p_data, int *size){

    if(!size[0])
        return;
    else{
        to[0][0] = reader(p_data, size);
        to[0]++;
        size[0]--;
        athena_tga_read_raw(to, reader, p_data, size);
    }
    
}

static void athena_tga_read_rle(uint32_t **to, athena_tga_read_data reader, const unsigned char **p_data, int *size){

    if(size[0]){
        const unsigned char *data = p_data[0],
            rle_value = data[0],
            num_pixels = rle_value & 0x7F;

        size[0]--;
        
        if(size[0]<=num_pixels)
            return;

        if(rle_value){
            const uint32_t color = reader(p_data, size);
            memset_pattern4(to[0], &color, num_pixels);
            to[0] += num_pixels;
        }
        else{
            int n = num_pixels;
            athena_tga_read_raw(to, reader, p_data, &n);
            size[0] -= num_pixels; 
        }
           
        athena_tga_read_rle(to, reader, p_data, size);
    }
}

unsigned Athena_LoadTGA(struct Athena_Image *to, const char *path){
    struct Athena_TGAImage tga_image;
    void (*image_reader)(uint32_t **, athena_tga_read_data, const unsigned char **, int *) = NULL;
    athena_tga_read_data pixel_reader = NULL;
    const unsigned char *data;
    int size;

    if(!to || !path)
        return ATHENA_LOADPNG_IS_NULL;

    data = BufferFile(path, &size);
    if(!data)
        return ATHENA_LOADPNG_NO_FILE;

    athena_tga_header_from_buffer(&tga_image, &data, &size);
    
    data += tga_image.header.id_length;
    size -= tga_image.header.id_length;
    
    if(tga_image.header.bits_per_pixel==32){
        pixel_reader = athena_tga_read_rgba32;
    }
    else if(tga_image.header.bits_per_pixel==24){
        pixel_reader = athena_tga_read_rgba24;
    }
    else {
        return ATHENA_LOADPNG_NFORMAT;
    }
    
    if(tga_image.header.compression){
        image_reader = athena_tga_read_rle;
    }
    else{
        image_reader = athena_tga_read_raw;
    }
    
    {
        int len = size;
        len/=(tga_image.header.bits_per_pixel/8);
        image_reader(&(to->pixels), pixel_reader, &data, &len);
    }

    FreeBufferFile((void *)data, size);
    
    return ATHENA_LOADPNG_SUCCESS;

}
