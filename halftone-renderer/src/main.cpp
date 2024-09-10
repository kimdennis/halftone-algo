#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <cmath> 
#include <vector>

// Safely load and convert image data
void safeLoadImage(const char* imagePath, unsigned char** imgData, int* width, int* height, int* channels) {
    *imgData = stbi_load(imagePath, width, height, channels, 3); // Load image as RGB (3 channels)
    if (!*imgData) {
        std::cerr << "Error loading image: " << imagePath << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Apply circular halftone dots to a single channel (e.g., R, G, B, or K)
void applyCircularHalftoneDots(unsigned char* data, int width, int height, int channels, int dotSizeFactor, int channelIndex) {
    float maxRadius = dotSizeFactor / 2.0f;   // Maximum dot radius

    for (int y = 0; y < height; y += dotSizeFactor) {
        for (int x = 0; x < width; x += dotSizeFactor) {
            // Sum the pixel values within the block (dot) for this channel
            int sum = 0;
            int pixelCount = 0;

            // Loop through the pixels in the block (dot)
            for (int dy = 0; dy < dotSizeFactor && (y + dy) < height; ++dy) {
                for (int dx = 0; dx < dotSizeFactor && (x + dx) < width; ++dx) {
                    int index = ((y + dy) * width + (x + dx)) * channels + channelIndex;
                    sum += data[index];
                    pixelCount++;
                }
            }

            // Calculate the average brightness for this block
            int avg = sum / pixelCount;

            // Map brightness to dot size (darker = larger dots, brighter = smaller dots)
            float brightness = avg / 255.0f;  // Normalize the value
            float radius = maxRadius * (1.0f - brightness);  // Larger radius for darker areas

            // Ensure a minimum dot size for bright areas
            radius = std::max(radius, 1.0f);

            // Draw the circular dot for this block
            for (int dy = -maxRadius; dy <= maxRadius; ++dy) {
                for (int dx = -maxRadius; dx <= maxRadius; ++dx) {
                    int distanceSquared = dx * dx + dy * dy;
                    int pixelX = x + dx;
                    int pixelY = y + dy;

                    if (pixelX >= 0 && pixelX < width && pixelY >= 0 && pixelY < height) {
                        int pixelIndex = (pixelY * width + pixelX) * channels + channelIndex;

                        if (distanceSquared <= radius * radius) {
                            // Inside the dot: Set the pixel to the average brightness
                            data[pixelIndex] = avg;
                        }
                        else {
                            // Outside the dot: Set the pixel to white (background)
                            data[pixelIndex] = 255;
                        }
                    }
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
    int dotSizeFactor = 13;

    // Apply the circular halftone effect to each color channel
    applyCircularHalftoneDots(imgData, width, height, channels, dotSizeFactor, 0);  // Red or Cyan 
    applyCircularHalftoneDots(imgData, width, height, channels, dotSizeFactor, 1);  // Green or Magenta 
    applyCircularHalftoneDots(imgData, width, height, channels, dotSizeFactor, 2);  // Blue or Yellow 

    // Save the result
    if (stbi_write_png("assets/output_halftone.png", width, height, channels, imgData, width * channels)) {
        std::cout << "Halftone effect completed and saved as output_halftone.png" << std::endl;
    }
    else {
        std::cerr << "Error saving image" << std::endl;
    }

    // Free the image memory
    stbi_image_free(imgData);

    return 0;
}
