#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <vector>
#include <algorithm> // For std::min and std::max

// Safely load and convert image data
void safeLoadImage(const char* imagePath, unsigned char** imgData, int* width, int* height, int* channels) {
    *imgData = stbi_load(imagePath, width, height, channels, 3); // Load image as RGB (3 channels)
    if (!*imgData) {
        std::cerr << "Error loading image: " << imagePath << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Convert RGB to YMCK
void rgbToYmck(unsigned char r, unsigned char g, unsigned char b, float& y, float& m, float& c, float& k) {
    // Convert to CMY
    float cTemp = 1.0f - (r / 255.0f);
    float mTemp = 1.0f - (g / 255.0f);
    float yTemp = 1.0f - (b / 255.0f);

    // Calculate the K (Black) component
    k = std::min(cTemp, std::min(mTemp, yTemp));

    // Adjust the CMY values by removing the K component
    if (k < 1.0f) {
        c = (cTemp - k) / (1.0f - k);
        m = (mTemp - k) / (1.0f - k);
        y = (yTemp - k) / (1.0f - k);
    }
    else {
        c = m = y = 0.0f;
    }
}

// Convert YMCK back to RGB
void ymckToRgb(float y, float m, float c, float k, unsigned char& r, unsigned char& g, unsigned char& b) {
    r = static_cast<unsigned char>((1.0f - (c * (1.0f - k) + k)) * 255.0f);
    g = static_cast<unsigned char>((1.0f - (m * (1.0f - k) + k)) * 255.0f);
    b = static_cast<unsigned char>((1.0f - (y * (1.0f - k) + k)) * 255.0f);
}

// Apply halftone dithering to a single YMCK channel
void applyHalftoneToChannel(unsigned char* data, int width, int height, int channels, int dotSizeFactor, int channelIndex) {
    for (int y = 0; y < height; y += dotSizeFactor) {
        for (int x = 0; x < width; x += dotSizeFactor) {
            int sum = 0;
            int pixelCount = 0;

            // Sum the pixel values within the block (dot)
            for (int dy = 0; dy < dotSizeFactor && (y + dy) < height; ++dy) {
                for (int dx = 0; dx < dotSizeFactor && (x + dx) < width; ++dx) {
                    int index = ((y + dy) * width + (x + dx)) * channels + channelIndex;
                    sum += data[index];
                    pixelCount++;
                }
            }

            // Calculate the average value for the block
            int avg = sum / pixelCount;

            // Determine the new pixel value (black or white) based on the average
            int newValue = (avg < 128) ? 0 : 255;

            // Set the new pixel values for the entire block
            for (int dy = 0; dy < dotSizeFactor && (y + dy) < height; ++dy) {
                for (int dx = 0; dx < dotSizeFactor && (x + dx) < width; ++dx) {
                    int index = ((y + dy) * width + (x + dx)) * channels + channelIndex;
                    data[index] = newValue;
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

    // Convert each pixel from RGB to YMCK
    std::vector<unsigned char> ymckData(width * height * 4); // 4 channels for YMCK
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 4;
            unsigned char r = imgData[(y * width + x) * channels];
            unsigned char g = imgData[(y * width + x) * channels + 1];
            unsigned char b = imgData[(y * width + x) * channels + 2];

            // Convert to YMCK
            float yVal, mVal, cVal, kVal;
            rgbToYmck(r, g, b, yVal, mVal, cVal, kVal);

            // Store the YMCK values
            ymckData[index] = static_cast<unsigned char>(yVal * 255.0f);
            ymckData[index + 1] = static_cast<unsigned char>(mVal * 255.0f);
            ymckData[index + 2] = static_cast<unsigned char>(cVal * 255.0f);
            ymckData[index + 3] = static_cast<unsigned char>(kVal * 255.0f);
        }
    }

    // Set the dot size factor (the larger the value, the larger the halftone dots)
    int dotSizeFactor = 1;

    // Apply halftone dithering to each YMCK channel
    applyHalftoneToChannel(ymckData.data(), width, height, 4, dotSizeFactor, 0);  // Yellow
    applyHalftoneToChannel(ymckData.data(), width, height, 4, dotSizeFactor, 1);  // Magenta
    applyHalftoneToChannel(ymckData.data(), width, height, 4, dotSizeFactor, 2);  // Cyan
    applyHalftoneToChannel(ymckData.data(), width, height, 4, dotSizeFactor, 3);  // Black (K)

    // Convert YMCK back to RGB
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 4;
            unsigned char yVal = ymckData[index];
            unsigned char mVal = ymckData[index + 1];
            unsigned char cVal = ymckData[index + 2];
            unsigned char kVal = ymckData[index + 3];

            // Convert back to RGB
            unsigned char r, g, b;
            ymckToRgb(yVal / 255.0f, mVal / 255.0f, cVal / 255.0f, kVal / 255.0f, r, g, b);

            // Store back in the original image data
            imgData[(y * width + x) * channels] = r;
            imgData[(y * width + x) * channels + 1] = g;
            imgData[(y * width + x) * channels + 2] = b;
        }
    }

    // Save the dithered result
    if (stbi_write_png("assets/output_dithered_ymck.png", width, height, channels, imgData, width * channels)) {
        std::cout << "Dithering completed and saved as output_dithered_ymck.png" << std::endl;
    }
    else {
        std::cerr << "Error saving image" << std::endl;
    }

    // Free the image memory
    stbi_image_free(imgData);

    return 0;
}
