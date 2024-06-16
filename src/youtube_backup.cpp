#include <stdio.h>

#include "../include/raylib.h"
#include "types.h"

#define WINDOW_WIDTH    1920
#define WINDOW_HEIGHT   1080

struct Header {
    u8 version;
    u32 size;
};

struct Content {
    Header header;
    void *data;
};

int main()
{
    u8 bytes[] = {
        0x10, 0x20, 0x30, 0x40,
    };

    for (int i = 0; i < sizeof(bytes); i++) {
        printf("0x%02x\n", bytes[i]);
    }

    Header header = {};
    header.version = 1;
    header.size = sizeof(bytes);

    Content content = {};
    content.header = header;
    content.data = bytes;

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Youtube backup");

    while (!WindowShouldClose()) {
        BeginDrawing();
            Color color = {};
            color.r = bytes[0];
            color.g = bytes[1];
            color.b = bytes[2];
            color.a = bytes[3];

            ClearBackground(BLACK);
            Rectangle rect = { 10, 10, 200, 200 };
            DrawRectangleRec(rect, color);
            DrawPixel(5, 5, color);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
