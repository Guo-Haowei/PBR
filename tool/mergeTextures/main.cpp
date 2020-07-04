#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
#include <iostream>
#include <assert.h>

// #define IMAGE_1 "Cerberus_A.png"
// #define IMAGE_2 "Cerberus_M.png"
#define IMAGE_1 "Cerberus_N.png"
#define IMAGE_2 "Cerberus_R.png"

int main()
{
    int width1, height1, channel1;
    int width2, height2, channel2;

    unsigned char* buffer1 = stbi_load(IMAGE_1, &width1, &height1, &channel1, 0);
    unsigned char* buffer2 = stbi_load(IMAGE_2, &width2, &height2, &channel2, 0);

    assert(width1 == width2);
    assert(height1 == height2);
    assert(channel1 == 3);
    assert(channel2 == 1);

    int size = width1 * height1;
    unsigned char* buffer = new unsigned char[4 * size];

    for (int i = 0; i < size; ++i)
    {
        buffer[4 * i] = buffer1[3 * i];
        buffer[4 * i + 1] = buffer1[3 * i + 1];
        buffer[4 * i + 2] = buffer1[3 * i + 2];
        buffer[4 * i + 3] = buffer2[i];
    }

    stbi_write_png("test.png", width1, height1, 4, buffer, 4 * width1);

    delete [] buffer;
}