#include <stdio.h>
#include <stdlib.h>

#include "../include/raylib.h"
#include "types.h"

// I can't include raylib and curl because some functions from windows socket API has the same name as some of raylib.
// TODO: maybe generate 2 object files and link them at the end. This way each binary would have their own copy of the conflicting methods.
// #include "../include/curl/curl.h"


#define WINDOW_WIDTH    1920
#define WINDOW_HEIGHT   1080

struct Content {
    u32 version;
    u32 size;
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

// void write_bitmap()
// {
//     u32 data[] = {
//         0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
//         0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
//         0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
//         0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,

//         0x00ff0000, 0x00ff0000, 0x00ff0000, 0x00ff0000, 0x00ff0000, 0x00ff0000,
//         0x00ff0000, 0x00ff0000, 0x00ff0000, 0x00ff0000, 0x00ff0000, 0x00ff0000,
//         0x00ff0000, 0x00ff0000, 0x00ff0000, 0x00ff0000, 0x00ff0000, 0x00ff0000,
//         0x00ff0000, 0x00ff0000, 0x00ff0000, 0x00ff0000, 0x00ff0000, 0x00ff0000,
//     };

//     BitmapHeader header = {};
//     header.signature = 0x4d42;
//     header.file_size = sizeof(Bitmap) + sizeof(data);
//     header.data_offset = sizeof(Bitmap);

//     BitmapInfoHeader info_header = {};
//     info_header.info_header_size = sizeof(BitmapInfoHeader);
//     info_header.width = 6;
//     info_header.height = 8;
//     info_header.planes = 1;
//     info_header.bits_per_pixel = 32;
//     info_header.compression = 3;
//     info_header.image_size = info_header.width*info_header.height*4;
//     info_header.horizontal_resolution = 2835;
//     info_header.vertical_resolution = 2835;
//     info_header.colors_used = 0;
//     info_header.important_colors = 0;
//     info_header.red_mask = 0xff000000;
//     info_header.green_mask = 0xff0000;
//     info_header.blue_mask = 0xff00;
//     info_header.alpha_mask = 0xff;

//     Bitmap bitmap = {};
//     bitmap.header = header;
//     bitmap.info_header = info_header;

//     char *filename = "test_write.bmp";
//     FILE *file;
//     if (fopen_s(&file, filename, "wb") != 0) {
//         exit(1);
//     }

//     fwrite(&bitmap, sizeof(bitmap), 1, file);
//     fwrite(data, sizeof(data), 1, file);

//     fclose(file);
// }

struct ContentWritable {
    u32 version;
    u32 size;
};

void write_bitmap(char *filename, Content content)
{
    u32 size = content.size;
    u32 width = 1920;
    u32 height = 1080;
    u32 image_size_bytes = width*height*4;
    u8 *data = content.data;

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

    FILE *file;
    if (fopen_s(&file, filename, "wb") != 0) {
        exit(1);
    }

    ContentWritable writable = {};
    writable.version = content.version;
    writable.size = content.size;

    fwrite(&bitmap, sizeof(Bitmap), 1, file);

    // Store data
    fwrite(&writable, sizeof(ContentWritable), 1, file);
    fwrite(data, size, 1, file);
    
    u8 empty_data[] = { 0 };
    u32 remaining_data_bytes = image_size_bytes - size - sizeof(ContentWritable);
    for (u32 i = 0; i < remaining_data_bytes; i++) {
        fwrite(empty_data, sizeof(empty_data), 1, file);
    }

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
    content.version = 1;
    
    fseek(file, 0, SEEK_END);
    content.size = ftell(file);
    fseek(file, 0, SEEK_SET);

    content.data = (u8 *)malloc(content.size);

    fread(content.data, content.size, 1, file);

    fclose(file);

    return content;
}

int main()
{
    Content content = get_file_content("test.txt");

    // read_bitmap();

    // write_bitmap();

    
    char *filename = "test_bitmap_from_file.bmp";
    write_bitmap(filename, content);

    read_bitmap(filename);
    
    u32 size_block = content.size / 4;
    u32 size_remaining = content.size - size_block;
    
#if 0
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Youtube backup");

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(BLACK);

            int w = 10;
            u64 block_index = 0;
            
            // Encode version
            {
                Color color = {};
                color.r = content.version;
                
                float x = (float)((block_index * w) % WINDOW_WIDTH);
                float y = (float)(((block_index * w) / WINDOW_WIDTH) * w);
                Rectangle rect = { x, y, (float)w, (float)w };
                DrawRectangleRec(rect, color);

                block_index++;
            }

            // Encode size - part 1
            {
                Color color = {};
                color.r = (content.size >> 0)  & 0xFF;
                color.g = (content.size >> 8)  & 0xFF;
                color.b = (content.size >> 16) & 0xFF;
                color.a = (content.size >> 24) & 0xFF;

                float x = (float)((block_index * w) % WINDOW_WIDTH);
                float y = (float)(((block_index * w) / WINDOW_WIDTH) * w);
                Rectangle rect = { x, y, (float)w, (float)w };
                DrawRectangleRec(rect, color);

                block_index++;
            }

            // Encode size - part 2
            // {
            //     Color color = {};
            //     color.r = (content.size >> 32) & 0xFF;
            //     color.g = (content.size >> 40) & 0xFF;
            //     color.b = (content.size >> 48) & 0xFF;
            //     color.a = (content.size >> 56) & 0xFF;

            //     float x = (float)((block_index * w) % WINDOW_WIDTH);
            //     float y = (float)(((block_index * w) / WINDOW_WIDTH) * w);
            //     Rectangle rect = { x, y, (float)w, (float)w };
            //     DrawRectangleRec(rect, color);

            //     block_index++;
            // }

            for (u64 i = 0; i < size_block; i++) {
                u64 data_index = 4*i;
                Color color = {};
                color.r = content.data[data_index];
                color.g = content.data[data_index + 1];
                color.b = content.data[data_index + 2];
                color.a = content.data[data_index + 3];

                float x = (float)((((block_index + i) * w)) % WINDOW_WIDTH);
                float y = (float)(((((block_index + i) * w)) / WINDOW_WIDTH) * w);
                Rectangle rect = { x, y, (float)w, (float)w };
                DrawRectangleRec(rect, color);
            }

        EndDrawing();
    }

    CloseWindow();
#endif

    return 0;
}
