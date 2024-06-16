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
    u8 version;
    u64 size;
    u8 *data;
};

int main()
{
    char *filename = "test.txt";
    // char *filename = "youtube_backup.pdb";

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

    
    u64 size_block = content.size / 4;
    u64 size_remaining = content.size - size_block;
    

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
            {
                Color color = {};
                color.r = (content.size >> 32) & 0xFF;
                color.g = (content.size >> 40) & 0xFF;
                color.b = (content.size >> 48) & 0xFF;
                color.a = (content.size >> 56) & 0xFF;

                float x = (float)((block_index * w) % WINDOW_WIDTH);
                float y = (float)(((block_index * w) / WINDOW_WIDTH) * w);
                Rectangle rect = { x, y, (float)w, (float)w };
                DrawRectangleRec(rect, color);

                block_index++;
            }

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

    return 0;
}
