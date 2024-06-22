#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

// I can't include raylib and curl because some functions from windows socket API has the same name as some of raylib.
// TODO: maybe generate 2 object files and link them at the end. This way each binary would have their own copy of the conflicting methods.
// #include "../include/curl/curl.h"


#if 1
#define IMAGE_WIDTH     1920
#define IMAGE_HEIGHT    1080
#else
#define IMAGE_WIDTH     50
#define IMAGE_HEIGHT    50
#endif

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

ContentHeader copy_header(ContentHeader *ptr)
{
    ContentHeader header = {};
    if (ptr) {
        header.version = ptr->version;
        header.size = ptr->size;
        strncpy(header.filename, ptr->filename, strlen(ptr->filename));
    }
    
    return header;
}

void get_data_from_file(char *filename, u8 *data)
{
    FILE *file;
    if (fopen_s(&file, filename, "rb") != 0) {
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    u32 file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    fread(data, file_size, 1, file);

    fclose(file);
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

void read_bitmap()
{
    size_t data_size = IMAGE_WIDTH*IMAGE_HEIGHT*4 + sizeof(ContentHeader);
    u8 *data = (u8 *)malloc(data_size);
    ContentHeader header = {};
    u8 *file_data = 0;
    u32 bytes_written = 0;
    FILE *output;

    for (int image_index = 0; ; image_index++) {
        memset(data, 0, data_size);

        char image_filename[128];
        snprintf(image_filename, sizeof(image_filename), image_filename_template, image_index);
        
        get_data_from_file(image_filename, data);

        Bitmap *bitmap = (Bitmap *)data;

        u32 bytes_read;
        if (image_index == 0) {
            header = copy_header((ContentHeader *)(data + bitmap->header.data_offset));
            file_data = data + bitmap->header.data_offset + sizeof(ContentHeader);
            bytes_read = MIN(bitmap->info_header.image_size - sizeof(ContentHeader), header.size);

            char output_filename[128];
            snprintf(output_filename, sizeof(output_filename), "output_%s", header.filename);

            if (fopen_s(&output, output_filename, "wb") != 0) {
                exit(1);
            }
        } else {
            file_data = data + bitmap->header.data_offset;
            bytes_read = MIN(bitmap->info_header.image_size, header.size - bytes_written);
        }
        
        fwrite(file_data, bytes_read, 1, output);

        bytes_written += bytes_read;
        if (bytes_written >= header.size) {
            break;
        }
    }

    if (output) {
        fclose(output);
    }

    free(data);
}

void write_bitmap(Content content)
{
    s32 width = IMAGE_WIDTH;
    s32 height = IMAGE_HEIGHT;
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

            // When there is only 1 image, the whole content fills in it, so do not subtract the content header because it will write the whole data.
            if (images_to_generate != 1) {
                bytes_to_write -= sizeof(ContentHeader);
            }
        }

        fwrite(content.data + offset, bytes_to_write, 1, file);

        
        // Store unfill image with 0.
        u8 empty_data[] = { 0x1 };

        for (s32 i = 0; i < image_left_fill_bytes; i++) {
            fwrite(empty_data, sizeof(empty_data), 1, file);
        }

        fclose(file);

        offset += bytes_to_write;
        remaining_data_size -= bytes_to_write;
    }
}

int main()
{
    Content content = get_file_content("test.txt");
    write_bitmap(content);

    read_bitmap();

    return 0;
}
