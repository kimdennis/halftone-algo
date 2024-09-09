#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <vector>

// Safely load and convert image data
void safeLoadImage(const char* imagePath, unsigned char** imgData, int* width, int* height, int* channels) {
    *imgData = stbi_load(imagePath, width, height, channels, 3); // Load image as RGB (3 channels)
    if (!*imgData) {
        std::cerr << "Error loading image: " << imagePath << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Modified halftone dithering algorithm that retains color
void applyHalftoneDithering(unsigned char* data, int width, int height, int channels, int dotSizeFactor) {
    for (int y = 0; y < height; y += dotSizeFactor) {
        for (int x = 0; x < width; x += dotSizeFactor) {
            // Sum the pixel values within the block (dot) for each color channel
            int sumR = 0, sumG = 0, sumB = 0;
            int pixelCount = 0;

            // Loop through the pixels in the block (dot)
            for (int dy = 0; dy < dotSizeFactor && (y + dy) < height; ++dy) {
                for (int dx = 0; dx < dotSizeFactor && (x + dx) < width; ++dx) {
                    int index = ((y + dy) * width + (x + dx)) * channels;
                    sumR += data[index];       // Red channel
                    sumG += data[index + 1];   // Green channel
                    sumB += data[index + 2];   // Blue channel
                    pixelCount++;
                }
            }

            // Calculate the average brightness of the block for each channel
            int avgR = sumR / pixelCount;
            int avgG = sumG / pixelCount;
            int avgB = sumB / pixelCount;

            // Determine the new pixel values (black or white for each channel) based on the average
            int newR = (avgR < 128) ? 0 : 255;
            int newG = (avgG < 128) ? 0 : 255;
            int newB = (avgB < 128) ? 0 : 255;

            // Set the new pixel values for the entire block
            for (int dy = 0; dy < dotSizeFactor && (y + dy) < height; ++dy) {
                for (int dx = 0; dx < dotSizeFactor && (x + dx) < width; ++dx) {
                    int index = ((y + dy) * width + (x + dx)) * channels;
                    data[index] = newR;       // Red channel
                    data[index + 1] = newG;   // Green channel
                    data[index + 2] = newB;   // Blue channel
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

    // Apply the modified halftone dithering that retains color
    applyHalftoneDithering(imgData, width, height, channels, dotSizeFactor);

    // Save the dithered result
    if (stbi_write_png("assets/output_dithered.png", width, height, channels, imgData, width * channels)) {
        std::cout << "Dithering completed and saved as output_dithered.png" << std::endl;
    }
    else {
        std::cerr << "Error saving image" << std::endl;
    }

    // Free the image memory
    stbi_image_free(imgData);

    return 0;
}
