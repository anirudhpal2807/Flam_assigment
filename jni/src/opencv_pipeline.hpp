#pragma once

#include <cstdint>
#include <cstddef>

namespace edgeviewer {

struct ImageView {
    const uint8_t* data;
    int width;
    int height;
    int stride; // bytes per row
    int channels; // e.g. 4 for RGBA
};

struct MutableImageView {
    uint8_t* data;
    int width;
    int height;
    int stride;
    int channels;
};

// Convert input RGBA to grayscale output (1 channel, packed 8-bit)
// If output buffer is null or too small, returns required size and false.
// On success, writes into outBuffer and returns true.
bool processGrayscale(const ImageView& inputRgba,
                      uint8_t* outBuffer,
                      size_t outBufferSize,
                      size_t& outBytesWritten);

// Apply Canny edge detection on input RGBA and write a single-channel mask
// into outBuffer.
bool processCannyEdges(const ImageView& inputRgba,
                       double lowThreshold,
                       double highThreshold,
                       uint8_t* outBuffer,
                       size_t outBufferSize,
                       size_t& outBytesWritten);

} // namespace edgeviewer


