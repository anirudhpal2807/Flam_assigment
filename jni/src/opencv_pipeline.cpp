#include "opencv_pipeline.hpp"

#include <algorithm>

namespace edgeviewer {

static inline size_t requiredGrayBytes(int width, int height) {
    return static_cast<size_t>(width) * static_cast<size_t>(height);
}

bool processGrayscale(const ImageView& inputRgba,
                      uint8_t* outBuffer,
                      size_t outBufferSize,
                      size_t& outBytesWritten) {
    if (inputRgba.data == nullptr || inputRgba.width <= 0 || inputRgba.height <= 0 || inputRgba.channels < 3) {
        outBytesWritten = 0;
        return false;
    }

    const size_t need = requiredGrayBytes(inputRgba.width, inputRgba.height);
    if (outBuffer == nullptr || outBufferSize < need) {
        outBytesWritten = need;
        return false;
    }

    // Minimal CPU fallback (no OpenCV yet): convert RGBA to gray using luma approximation.
    // Gray = 0.299 R + 0.587 G + 0.114 B
    const uint8_t* srcRow = inputRgba.data;
    uint8_t* dst = outBuffer;
    const int srcStride = (inputRgba.stride > 0) ? inputRgba.stride : inputRgba.width * inputRgba.channels;
    for (int y = 0; y < inputRgba.height; ++y) {
        const uint8_t* src = srcRow;
        for (int x = 0; x < inputRgba.width; ++x) {
            const int idx = x * inputRgba.channels;
            const uint8_t r = src[idx + 0];
            const uint8_t g = src[idx + 1];
            const uint8_t b = src[idx + 2];
            const int gray = static_cast<int>(0.299f * r + 0.587f * g + 0.114f * b + 0.5f);
            *dst++ = static_cast<uint8_t>(std::clamp(gray, 0, 255));
        }
        srcRow += srcStride;
    }

    outBytesWritten = need;
    return true;
}

bool processCannyEdges(const ImageView& inputRgba,
                       double /*lowThreshold*/,
                       double /*highThreshold*/,
                       uint8_t* outBuffer,
                       size_t outBufferSize,
                       size_t& outBytesWritten) {
    // Placeholder: produce a grayscale mask identical to processGrayscale.
    // Will be replaced with real OpenCV Canny in a later commit.
    return processGrayscale(inputRgba, outBuffer, outBufferSize, outBytesWritten);
}

} // namespace edgeviewer


