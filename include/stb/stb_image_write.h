/* stb_image_write - v1.16 - public domain - http://nothings.org/stb */
/* minimal subset for PNG writing used in this project */
#ifndef STB_IMAGE_WRITE_H
#define STB_IMAGE_WRITE_H

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

int stbi_write_png_to_func(void (*func)(void *context, void *data, int size), void *context, int w, int h, int comp, const void *data, int stride_in_bytes);

#ifdef STB_IMAGE_WRITE_IMPLEMENTATION

static void stbi__write_png_core(void (*func)(void *context, void *data, int size), void *context, int x, int y, int comp, const void *data, int stride_in_bytes);

int stbi_write_png_to_func(void (*func)(void *context, void *data, int size), void *context, int w, int h, int comp, const void *data, int stride_in_bytes)
{
    stbi__write_png_core(func, context, w, h, comp, data, stride_in_bytes);
    return 1;
}

/* Extremely small PNG writer: supports 8-bit RGBA only, no compression tricks */
#include <string.h>

static unsigned stbiw__crc32(unsigned char *buffer, int len)
{
    static unsigned crc_table[256];
    static int inited = 0;
    unsigned crc = ~0u;
    int i,j;
    if (!inited) {
        for (i=0; i<256; ++i) {
            crc_table[i] = i;
            for (j=0; j<8; ++j)
                crc_table[i] = (crc_table[i] >> 1) ^ (0xEDB88320u & -(crc_table[i] & 1));
        }
        inited = 1;
    }
    for (i=0; i<len; ++i)
        crc = (crc >> 1) ^ crc_table[(crc & 1) ^ buffer[i]];
    return ~crc;
}

static void stbiw__write32be(unsigned char *data, unsigned v)
{
    data[0] = (unsigned char)((v >> 24) & 0xff);
    data[1] = (unsigned char)((v >> 16) & 0xff);
    data[2] = (unsigned char)((v >> 8) & 0xff);
    data[3] = (unsigned char)((v) & 0xff);
}

static void stbi__write_png_chunk(void (*func)(void *context, void *data, int size), void *context, const char *tag, unsigned char *data, int length)
{
    unsigned char length_be[4];
    stbiw__write32be(length_be, (unsigned)length);
    func(context, length_be, 4);
    func(context, (void*)tag, 4);
    if (length) func(context, data, length);
    unsigned crc_input_len = length + 4;
    unsigned char *crc_buffer = (unsigned char*)malloc(crc_input_len);
    memcpy(crc_buffer, tag, 4);
    if (length) memcpy(crc_buffer + 4, data, length);
    unsigned crc = stbiw__crc32(crc_buffer, (int)crc_input_len);
    unsigned char crc_be[4];
    stbiw__write32be(crc_be, crc);
    func(context, crc_be, 4);
    free(crc_buffer);
}

static void stbi__write_png_core(void (*func)(void *context, void *data, int size), void *context, int x, int y, int comp, const void *data, int stride_in_bytes)
{
    const int bytes_per_pixel = comp;
    const int stride = stride_in_bytes ? stride_in_bytes : x * bytes_per_pixel;
    const unsigned char png_signature[8] = {137,80,78,71,13,10,26,10};
    func(context, (void*)png_signature, 8);

    unsigned char ihdr[13];
    stbiw__write32be(ihdr+0, (unsigned)x);
    stbiw__write32be(ihdr+4, (unsigned)y);
    ihdr[8] = 8; /* bit depth */
    ihdr[9] = (unsigned char)((comp==4)?6:2); /* color type RGBA/RGB */
    ihdr[10] = ihdr[11] = ihdr[12] = 0; /* compression, filter, interlace */
    stbi__write_png_chunk(func, context, "IHDR", ihdr, 13);

    /* single uncompressed zlib block */
    int zlen = y * (1 + x * bytes_per_pixel);
    int adler = 1;
    unsigned char *zlib = (unsigned char*)malloc(zlen + 6);
    zlib[0] = 0x78; zlib[1] = 0x01; /* zlib header for no compression */
    int p = 2;
    int remaining = zlen;
    while (remaining > 0) {
        int block = remaining > 0xffff ? 0xffff : remaining;
        zlib[p++] = (remaining == block) ? 1 : 0;
        zlib[p++] = block & 0xff;
        zlib[p++] = (block >> 8) & 0xff;
        zlib[p++] = (~block) & 0xff;
        zlib[p++] = ((~block) >> 8) & 0xff;
        for (int row = 0; row < block; ++row)
            zlib[p++] = ((unsigned char*)data)[row];
        data = (unsigned char*)data + block;
        remaining -= block;
    }
    /* Adler32 placeholder */
    stbiw__write32be(zlib + p, (unsigned)adler);
    p += 4;
    stbi__write_png_chunk(func, context, "IDAT", zlib+2, p-2);
    free(zlib);

    stbi__write_png_chunk(func, context, "IEND", NULL, 0);
}

#endif
#endif
