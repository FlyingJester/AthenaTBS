#include "image.h"
#include "bufferfile/bufferfile.h"
#include "memset_pattern4.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
    Technically the largest possible TGA image would need 35 bits for the field itself.
    It would consist of a 0xFFFF by 0xFFFF field, encoded with RLE, but with each pixel
    encoded as not-RLE, and in runs of 1 only.
    
    I don't particularly care. Such a TGA is highly unlikely, as it would require either
    an entirely incompetent encoder (using RLE-encoding but refusing to actually encode
    runs) or a moderately competent encoder (it's easier to encode each non-RLE pixel as
    an individual run of 1 pixel) and a very odd TGA.
    
    We don't handle that. Actually, we refuse to open a TGA with a width or height of 
    more than 0xFFFE, which precludes anything higher than a 33-bit map.
    
    We include all sizes as 32-bit values since the need for more bits is a seriously
    edge case, and we are probably being fed garbage data (or utterly poorly encoded
    real data) if we encounter a larger field.
*/

enum Athena_TGAFormat { Nothing, RGBA, Mapped, BlackWhite };

struct Athena_TGAHeader {
    const char *id;
    uint8_t id_length;
    unsigned is_rle, has_map;
    enum Athena_TGAFormat format;

    const uint8_t *color_map;
    uint16_t color_map_entries;
    uint16_t x_origin, y_origin, w, h;
    
    uint8_t bits_per_map_entry, bits_per_pixel;
    uint8_t flipped_vertically;

};

/* return how many bytes read */
typedef uint8_t (*athena_tga_pixel_reader)(uint32_t *to, const uint8_t *from);

static uint16_t athena_lo_hi_short(const void *z_){
    const uint8_t *z = z_;
    uint16_t r = z[1];
    r <<= 8;
    r |= z[0];
    return r;
}

/* All reader functions return how many bytes they have consumed. A value of zero indicates an error. 
 * The accumulator exists because Solaris Studio will not perform TCO if we just put the addition on
 * the result of the next call.
 */
 
static uint8_t athena_read_tga_black_and_white(uint32_t *to, const uint8_t *from){
    to[0] = Athena_RGBAToRaw(*from, *from, *from, 0xFF);
    return 1;
}

static uint8_t athena_read_tga_15(uint32_t *to, const uint8_t *from){
    const uint16_t concat = athena_lo_hi_short(from);
    to[0] = Athena_RGBAToRaw(
        ((concat >> 10) & 0x1F) << 3, 
        ((concat >> 5)  & 0x1F) << 3, 
         (concat & 0x1F) << 3, 
         0xFF);
    return 2;
}

static uint8_t athena_read_tga_16(uint32_t *to, const uint8_t *from){
    const uint16_t concat = athena_lo_hi_short(from);
    to[0] = Athena_RGBAToRaw(
        ((concat >> 10) & 0x1F) << 3, 
        ((concat >> 5)  & 0x1F) << 3, 
         (concat & 0x1F) << 3, 
        ((concat & 0x80) == 0x80) ? 0xFF : 0);
    return 2;
}

static uint8_t athena_read_tga_24(uint32_t *to, const uint8_t *from){
    to[0] = Athena_RGBAToRaw(from[0], from[1], from[2], 0xFF);
    return 3;
}

static uint8_t athena_read_tga_32(uint32_t *to, const uint8_t *from){
    to[0] = Athena_RGBAToRaw(from[0], from[1], from[2], from[3]);
    return 4;
}

static uint32_t athena_tga_read_raw(struct Athena_Image *to, uint16_t x, uint16_t y, const uint8_t *field, athena_tga_pixel_reader pixel_reader, uint32_t accumulator){
    if(x >= to->w)
        return athena_tga_read_raw(to, 0, y + 1, field, pixel_reader, accumulator);
    else if(y >= to->h)
        return accumulator;
    else{
        const uint32_t z = pixel_reader(Athena_Pixel(to, x, y), field);
        return athena_tga_read_raw(to, x + 1, y, field + z, pixel_reader, accumulator + 1);
    }
}

static int athena_tga_read_raw_inner(uint32_t *to, const uint8_t *field, unsigned pixels_remaining, athena_tga_pixel_reader pixel_reader, unsigned accumulator){
    if(pixels_remaining){
        const int size = pixel_reader(to, field);
        return athena_tga_read_raw_inner(to + 1, field + size, pixels_remaining - 1, pixel_reader, accumulator + size);
    }
    else{
        return accumulator;
    }
}

static uint32_t athena_tga_read_rle(struct Athena_Image *to, uint16_t x, uint16_t y, const uint8_t *field, athena_tga_pixel_reader pixel_reader, uint32_t accumulator){
    if(x >= to->w)
        return athena_tga_read_rle(to, x - to->w, y + 1, field, pixel_reader, accumulator);
    else if(y >= to->h)
        return accumulator;
    else{
        const uint8_t run_size = (field[0] & 0x7F) + 1,
            rle_packet = field[0] & 0x80;
        field ++;
        if(!rle_packet){
            const int size = athena_tga_read_raw_inner(Athena_Pixel(to, x, y), field, run_size, pixel_reader, 0);
            return athena_tga_read_rle(to, x + run_size, y, field + size, pixel_reader, accumulator + 1 + size);
        }
        else{
            uint32_t pattern;
            const int size = pixel_reader(&pattern, field);
            
            memset_pattern4(Athena_Pixel(to, x, y), &pattern, run_size << 2);
            return athena_tga_read_rle(to, x + run_size, y, field + size, pixel_reader, accumulator + 1 + size);
        }
        
    }
}

static int athena_tga_header_from_buffer(struct Athena_TGAHeader *header, const uint8_t *buffer){

    header->id = (char *)buffer + 0x12;
    header->id_length = buffer[0x00];
    header->has_map   = buffer[0x01] != 0;
    header->is_rle    = (buffer[0x02] & 8) != 0;
    header->format    = buffer[0x02] & 3;

    header->color_map = buffer + athena_lo_hi_short(buffer + 0x03);
    header->color_map_entries = athena_lo_hi_short(buffer + 0x05);
    header->bits_per_map_entry = buffer[0x07];

    header->x_origin = athena_lo_hi_short(buffer + 0x08);
    header->y_origin = athena_lo_hi_short(buffer + 0x0A);
    header->w = athena_lo_hi_short(buffer + 0x0C);
    header->h = athena_lo_hi_short(buffer + 0x0E);
    
    header->bits_per_pixel = buffer[0x10];
    
    header->flipped_vertically = !(buffer[0x11] & 0x20);
    
    return 0;
}

unsigned Athena_LoadTGA(struct Athena_Image *to, const char *path){
    int size;
    void * data = NULL;
    
    if(!to || !path)
        return ATHENA_LOADPNG_IS_NULL;
    
    data = BufferFile(path, &size);
    
    if(!data){
        return ATHENA_LOADPNG_NO_FILE;
    }
    else if(size < 0x12){
        FreeBufferFile(data, size);  
        return ATHENA_LOADPNG_BADFILE;  
    }
    else{
        struct Athena_TGAHeader header;
        athena_tga_pixel_reader pixel_reader;

        athena_tga_header_from_buffer(&header, data);

        switch(header.bits_per_pixel){
            case 8:
                pixel_reader = athena_read_tga_black_and_white;
                break;
            case 15:
                pixel_reader = athena_read_tga_15;
                break;
            case 16:
                pixel_reader = athena_read_tga_16;
                break;
            case 24:
                pixel_reader = athena_read_tga_24;
                break;
            case 32:
                pixel_reader = athena_read_tga_32;
                break;
            default:
                FreeBufferFile(data, size);
                return ATHENA_LOADPNG_NFORMAT;
        }

        Athena_CreateImage(to, header.w, header.h);

        {
            const uint8_t *field = ((const uint8_t *)header.id) + header.id_length;
            
            if(header.is_rle){
                athena_tga_read_rle(to, 0, 0, field, pixel_reader, 0);
            }
            else{
                athena_tga_read_raw(to, 0, 0, field, pixel_reader, 0);
            }
        }
        
        FreeBufferFile(data, size);   

        /* TGA is upside down. So we need to reverse it. */
        if(header.flipped_vertically)
            Athena_FlipImageVertically(to, to);
    }

    return ATHENA_LOADPNG_SUCCESS;
}
