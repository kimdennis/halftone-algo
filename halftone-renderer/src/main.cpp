#define STB_IMAGE_IMPLEMENTATION    // Implement stb_image.h
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION  // Implement stb_image_write.h
#include "stb_image_write.h"

#include <iostream>
#include <vector>    // For using heap-allocated memory for large data
#include <windows.h> // For Windows-specific functions

// Safe wrapper around sprintf to avoid deprecation warnings
#ifdef _MSC_VER
    #define snprintf sprintf_s
#else
    #define snprintf snprintf
#endif

// Safely load and convert image data
void safeLoadImage(const char* imagePath, unsigned char** imgData, int* width, int* height, int* channels) {
    *imgData = stbi_load(imagePath, width, height, channels, 1); // Load image as grayscale
    if (!*imgData) {
        std::cerr << "Error loading image: " << imagePath << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Floyd-Steinberg dithering algorithm with heap allocation for large data
void applyFloydSteinbergDithering(unsigned char* data, int width, int height, int channels) {
    std::vector<int> errorBuffer(width * height * channels, 0); // Allocate on heap
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * channels;
            int oldPixel = data[index];
            int newPixel = oldPixel < 128 ? 0 : 255;
            data[index] = newPixel;
            int error = oldPixel - newPixel;

            if (x + 1 < width)
                data[(y * width + (x + 1)) * channels] += error * 7 / 16;
            if (x - 1 >= 0 && y + 1 < height)
                data[((y + 1) * width + (x - 1)) * channels] += error * 3 / 16;
            if (y + 1 < height)
                data[((y + 1) * width + x) * channels] += error * 5 / 16;
            if (x + 1 < width && y + 1 < height)
                data[((y + 1) * width + (x + 1)) * channels] += error * 1 / 16;
        }
    }
}

int main() {
    // Load the image safely
    unsigned char* imgData = nullptr;
    int width, height, channels;
    safeLoadImage("assets/input.png", &imgData, &width, &height, &channels);

    // Apply dithering
    applyFloydSteinbergDithering(imgData, width, height, 1);

    // Save the dithered result
    if (stbi_write_png("assets/output_dithered.png", width, height, 1, imgData, width)) {
        std::cout << "Dithering completed and saved as output_dithered.png" << std::endl;
    }
    else {
        std::cerr << "Error saving image" << std::endl;
    }

    // Free the image memory
    stbi_image_free(imgData);

    return 0;
}
