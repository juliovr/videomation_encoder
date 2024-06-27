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

#define BYTES_PER_PIXEL 3
#define FPS             30
#define MIN_FRAMES      (FPS)       /* Make the video at least 1 second long by generating the same amount of images as 1 FPS, otherwise youtube reports a check error. */

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

ContentHeader extract_file_from_bitmaps()
{
    size_t data_size = IMAGE_WIDTH*IMAGE_HEIGHT*BYTES_PER_PIXEL + sizeof(Bitmap);
    u8 *data = (u8 *)malloc(data_size);
    ContentHeader header = {};
    u8 *file_data = 0;
    u32 bytes_written = 0;
    FILE *output;
    ContentHeader result = {};

    for (int image_index = 0; ; image_index++) {
        memset(data, 0, data_size);

        char image_filename[128];
        snprintf(image_filename, sizeof(image_filename), "extracted_%04d.bmp", (image_index + 1));
        
        get_data_from_file(image_filename, data);

        Bitmap *bitmap = (Bitmap *)data;

        u32 bytes_read;
        if (image_index == 0) {
            header = copy_header((ContentHeader *)(data + bitmap->header.data_offset));
            file_data = data + bitmap->header.data_offset + sizeof(ContentHeader);
            bytes_read = MIN(bitmap->info_header.image_size - sizeof(ContentHeader), header.size);

            char output_filename[128];
            snprintf(output_filename, sizeof(output_filename), "output_%s", header.filename);

            strncpy(result.filename, output_filename, strlen(output_filename));

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

    return result;
}

void encode_data_to_bitmap(char *filename)
{
    Content content = get_file_content(filename);

    s32 width = IMAGE_WIDTH;
    s32 height = IMAGE_HEIGHT;
    s32 image_size_bytes = width*height*BYTES_PER_PIXEL;

    BitmapHeader header = {};
    header.signature = 0x4d42;
    header.file_size = sizeof(Bitmap) + image_size_bytes;
    header.data_offset = sizeof(Bitmap);

    BitmapInfoHeader info_header = {};
    info_header.info_header_size = sizeof(BitmapInfoHeader);
    info_header.width = width;
    info_header.height = height;
    info_header.planes = 1;
    info_header.bits_per_pixel = BYTES_PER_PIXEL*8;
    info_header.compression = 0;
    info_header.image_size = image_size_bytes;
    info_header.horizontal_resolution = 0;
    info_header.vertical_resolution = 0;
    info_header.colors_used = 0;
    info_header.important_colors = 0;
    
    Bitmap bitmap = {};
    bitmap.header = header;
    bitmap.info_header = info_header;

    char image_filename[128];

    int running_bytes_written = 0;

    int images_to_generate = ((sizeof(ContentHeader) + content.header.size) / image_size_bytes) + 1; // + 1 for int division (to round up).
    images_to_generate = MAX(images_to_generate, MIN_FRAMES);
    int offset = 0;
    int remaining_data_size = content.header.size;

    for (int image_index = 0; image_index < images_to_generate; image_index++) {
        snprintf(image_filename, sizeof(image_filename), image_filename_template, image_index);
        FILE *file;
        if (fopen_s(&file, image_filename, "wb") != 0) {
            exit(1);
        }

        fwrite(&bitmap, sizeof(Bitmap), 1, file);

        // Store data
        s32 bytes_to_write = MIN(image_size_bytes, remaining_data_size);
        s32 image_left_fill_bytes = image_size_bytes - bytes_to_write;

        // This only goes in the first image
        if (image_index == 0) {
            image_left_fill_bytes -= sizeof(ContentHeader);
            
            fwrite(&content.header, sizeof(ContentHeader), 1, file);

            // When the data does not fill in 1 image, subtract the content header because it won't fill in the image. It will be written in the next
            if (bytes_to_write > image_left_fill_bytes) {
                bytes_to_write -= sizeof(ContentHeader);
            }
        }

        if (bytes_to_write > 0) {
            fwrite(content.data + offset, bytes_to_write, 1, file);
        }

        
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

void create_video(char *video_filename)
{
    char command[256];
    snprintf(command, sizeof(command), "ffmpeg -y -framerate 30 -i %s -c:v libx264rgb -vf \"fps=30\" -crf 0 -preset veryslow -pix_fmt bgr24 %s", image_filename_template, video_filename);

    int rc = system(command);
    if (rc != 0) {
        exit(1);
    }
}

void extract_images_from_video(char *video_filename)
{
    char command[256];
    snprintf(command, sizeof(command), "ffmpeg -i %s -vf \"fps=30\" -vsync 0 -pix_fmt bgr24 extracted_%%04d.bmp", video_filename);

    int rc = system(command);
    if (rc != 0) {
        exit(1);
    }
}

Content debug_read_bitmap(char *filename)
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

    Content content = {};
    content.header.size = file_size;
    content.data = data;

    return content;
}

void compare_files(char *filename1, char *filename2)
{
    Content content1 = get_file_content(filename1);
    Content content2 = get_file_content(filename2);

    if (content1.header.size != content2.header.size) {
        fprintf(stderr, "Different file sizes\n");
        exit(1);
    }

    int match = 1;
    for (u32 i = 0; i < content1.header.size; i++) {
        if (content1.data[i] != content2.data[i]) {
            fprintf(stderr, "Byte %02x is different: %02x -> %02x\n", i, content1.data[i], content2.data[i]);
            match = 0;
        }
    }

    if (match) {
        printf("Files matched\n");
    } else {
        printf("Files doesn't matched\n");
    }
}

int main()
{
    char *input_filename = "test.txt";
    printf("Starting...\n");
    encode_data_to_bitmap(input_filename);
    
    create_video("output.mp4");
    extract_images_from_video("output.mp4");

    ContentHeader header = extract_file_from_bitmaps();

    compare_files(input_filename, header.filename);

    printf("Done\n");

    return 0;
}
