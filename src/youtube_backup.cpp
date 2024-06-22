#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/raylib.h"
#include "types.h"

// I can't include raylib and curl because some functions from windows socket API has the same name as some of raylib.
// TODO: maybe generate 2 object files and link them at the end. This way each binary would have their own copy of the conflicting methods.
// #include "../include/curl/curl.h"


#define IMAGE_WIDTH    1920
#define IMAGE_HEIGHT   1080

#define MAX(a, b) ((a) > (b)) ? (a) : (b)
#define MIN(a, b) ((a) < (b)) ? (a) : (b)

struct ContentHeader {
    u32 version;
    u32 size;
    char filename[128];
};

struct Content {
    ContentHeader header;
    u8 *data;
};

#pragma pack(push, 1)
struct BitmapHeader {
    u16 signature;
    u32 file_size;
    u16 reserved_1;
    u16 reserved_2;
    u32 data_offset;
};

struct BitmapInfoHeader {
    u32 info_header_size;
    s32 width;
    s32 height;
    u16 planes;
    u16 bits_per_pixel;
    u32 compression;
    u32 image_size;
    s32 horizontal_resolution;
    s32 vertical_resolution;
    u32 colors_used;
    u32 important_colors;
    u32 red_mask;
    u32 green_mask;
    u32 blue_mask;
    u32 alpha_mask;
};


struct Bitmap {
    BitmapHeader header;
    BitmapInfoHeader info_header;
};
#pragma pack(pop)

char *image_filename_template = "image_%04d.bmp";

void read_bitmap(char *filename)
{
    FILE *file;
    if (fopen_s(&file, filename, "rb") != 0) {
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    u32 file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    u8 *data = (u8 *)malloc(file_size);

    fread(data, file_size, 1, file);

    fclose(file);

    Bitmap *bitmap = (Bitmap *)data;
    int x = 5;
}

void write_bitmap(Content content)
{
#if 0
    s32 width = IMAGE_WIDTH;
    s32 height = IMAGE_HEIGHT;
#else
    s32 width = 50;
    s32 height = 50;
#endif
    s32 image_size_bytes = width*height*4;

    BitmapHeader header = {};
    header.signature = 0x4d42;
    header.file_size = sizeof(Bitmap) + image_size_bytes;
    header.data_offset = sizeof(Bitmap);

    BitmapInfoHeader info_header = {};
    info_header.info_header_size = sizeof(BitmapInfoHeader);
    info_header.width = width;
    info_header.height = height;
    info_header.planes = 1;
    info_header.bits_per_pixel = 32;
    info_header.compression = 3;
    info_header.image_size = image_size_bytes;
    info_header.horizontal_resolution = 2835;
    info_header.vertical_resolution = 2835;
    info_header.colors_used = 0;
    info_header.important_colors = 0;
    info_header.red_mask   = 0xff000000;
    info_header.green_mask = 0x00ff0000;
    info_header.blue_mask  = 0x0000ff00;
    info_header.alpha_mask = 0x000000ff;

    Bitmap bitmap = {};
    bitmap.header = header;
    bitmap.info_header = info_header;

    char filename[128];

    int running_bytes_written = 0;

    int images_to_generate = ((sizeof(ContentHeader) + content.header.size) / image_size_bytes) + 1; // + 1 for int division (to round up).
    int offset = 0;
    int remaining_data_size = content.header.size;

    for (int image_index = 0; image_index < images_to_generate; image_index++) {
        snprintf(filename, sizeof(filename), image_filename_template, image_index);
        FILE *file;
        if (fopen_s(&file, filename, "wb") != 0) {
            exit(1);
        }

        fwrite(&bitmap, sizeof(Bitmap), 1, file);

        // Store data
        u32 bytes_to_write = MIN(image_size_bytes, remaining_data_size);
        s32 image_left_fill_bytes = image_size_bytes - bytes_to_write;

        // This only goes in the first image
        if (image_index == 0) {
            fwrite(&content.header, sizeof(ContentHeader), 1, file);
            bytes_to_write -= sizeof(ContentHeader);
        }

        fwrite(content.data + offset, bytes_to_write, 1, file);

        
        // Store unfill image with 0.
        u8 empty_data[] = { 0 };

        for (s32 i = 0; i < image_left_fill_bytes; i++) {
            fwrite(empty_data, sizeof(empty_data), 1, file);
        }

        fclose(file);

        offset += bytes_to_write;
        remaining_data_size -= bytes_to_write;
    }
}

Content get_file_content(char *filename)
{
    FILE *file;
    errno_t fopen_error = fopen_s(&file, filename, "rb");
    if (fopen_error != 0) {
        exit(1);
    }

    Content content = {};
    content.header.version = 1;
    strncpy(content.header.filename, filename, strlen(filename));
    
    fseek(file, 0, SEEK_END);
    content.header.size = ftell(file);
    fseek(file, 0, SEEK_SET);

    content.data = (u8 *)malloc(content.header.size);

    fread(content.data, content.header.size, 1, file);

    fclose(file);

    return content;
}

int main()
{
    Content content = get_file_content("test.txt");
    write_bitmap(content);

    return 0;
}
