#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <vector>    // For using heap-allocated memory for large data

// Safely load and convert image data
void safeLoadImage(const char* imagePath, unsigned char** imgData, int* width, int* height, int* channels) {
    *imgData = stbi_load(imagePath, width, height, channels, 1); // Load image as grayscale
    if (!*imgData) {
        std::cerr << "Error loading image: " << imagePath << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Modified Floyd-Steinberg dithering with block size (dot size control)
void applyHalftoneDithering(unsigned char* data, int width, int height, int channels, int dotSizeFactor) {
    for (int y = 0; y < height; y += dotSizeFactor) {
        for (int x = 0; x < width; x += dotSizeFactor) {
            // Average the pixel values within the block (dot)
            int sum = 0;
            int pixelCount = 0;

            // Loop through the pixels in the block
            for (int dy = 0; dy < dotSizeFactor && (y + dy) < height; ++dy) {
                for (int dx = 0; dx < dotSizeFactor && (x + dx) < width; ++dx) {
                    int index = ((y + dy) * width + (x + dx)) * channels;
                    sum += data[index];
                    pixelCount++;
                }
            }

            // Calculate the average brightness of the block
            int average = sum / pixelCount;

            // Determine the new pixel value (black or white) based on the average
            int newPixel = (average < 128) ? 0 : 255;

            // Set the new pixel value for the entire block
            for (int dy = 0; dy < dotSizeFactor && (y + dy) < height; ++dy) {
                for (int dx = 0; dx < dotSizeFactor && (x + dx) < width; ++dx) {
                    int index = ((y + dy) * width + (x + dx)) * channels;
                    data[index] = newPixel;
                }
            }
        }
    }
}

int main() {
    // Load the image safely
    unsigned char* imgData = nullptr;
    int width, height, channels;
    safeLoadImage("assets/input.png", &imgData, &width, &height, &channels);

    // Set the dot size factor (the larger the value, the larger the halftone dots)
    int dotSizeFactor = 1;  // Increase this value to make the dots bigger

    // Apply the modified halftone dithering
    applyHalftoneDithering(imgData, width, height, 1, dotSizeFactor);

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
