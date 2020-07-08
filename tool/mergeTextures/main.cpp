#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
#include <iostream>
#include <assert.h>

void merge(const char* image1, const char* image2)
{
    int width1, height1, channel1;
    int width2, height2, channel2;

    unsigned char* buffer1 = stbi_load(image1, &width1, &height1, &channel1, 0);
    unsigned char* buffer2 = stbi_load(image2, &width2, &height2, &channel2, 0);

    assert(width1 == width2);
    assert(height1 == height2);
    assert(channel1 == 3);
    std::cout << channel2 << std::endl;

    int size = width1 * height1;
    unsigned char* buffer = new unsigned char[4 * size];

    for (int i = 0; i < size; ++i)
    {
        buffer[4 * i + 0] = buffer1[channel1 * i];
        buffer[4 * i + 1] = buffer1[channel1 * i + 1];
        buffer[4 * i + 2] = buffer1[channel1 * i + 2];
        buffer[4 * i + 3] = buffer2[channel2 * i];
    }

    stbi_write_png("test.png", width1, height1, 4, buffer, 4 * width1);

    delete [] buffer;
}

int main()
{
    // merge("E.jpg", "AO.jpg");
    // int width, height, channel;
    // unsigned char* buffer1 = stbi_load("Emissive.jpg", &width, &height, &channel, 0);

    int width = 1024;
    int height = 1024;

    int size = width * height;
    unsigned char* buffer = new unsigned char[4 * size];

    int color = 2; // green

    for (int i = 0; i < size; ++i)
    {
        buffer[4 * i + 0] = 0;
        buffer[4 * i + 1] = 0;
        buffer[4 * i + 2] = 0;
        buffer[4 * i + 3] = 255; // one
    }

    stbi_write_png("test2.png", width, height, 4, buffer, 4 * width);

    delete [] buffer;
}