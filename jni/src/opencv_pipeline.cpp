#include "opencv_pipeline.hpp"

#include <algorithm>
#include <vector>

// Enable OpenCV Canny when OpenCV is available and this macro is defined via build flags
#ifdef EDGEVIEWER_USE_OPENCV
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#endif

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
                       double lowThreshold,
                       double highThreshold,
                       uint8_t* outBuffer,
                       size_t outBufferSize,
                       size_t& outBytesWritten) {
#ifdef EDGEVIEWER_USE_OPENCV
    if (inputRgba.data == nullptr || inputRgba.width <= 0 || inputRgba.height <= 0 || inputRgba.channels < 3) {
        outBytesWritten = 0;
        return false;
    }

    const size_t need = requiredGrayBytes(inputRgba.width, inputRgba.height);
    if (outBuffer == nullptr || outBufferSize < need) {
        outBytesWritten = need;
        return false;
    }

    // Wrap input RGBA
    const int type = (inputRgba.channels == 4) ? CV_8UC4 : CV_8UC3;
    cv::Mat src(inputRgba.height, inputRgba.width, type, const_cast<uint8_t*>(inputRgba.data), inputRgba.stride);
    cv::Mat gray, edges;
    if (type == CV_8UC4) {
        cv::cvtColor(src, gray, cv::COLOR_RGBA2GRAY);
    } else {
        cv::cvtColor(src, gray, cv::COLOR_RGB2GRAY);
    }
    cv::Canny(gray, edges, lowThreshold, highThreshold, 3, true);

    // Copy to output (single-channel)
    if (static_cast<size_t>(edges.total()) != need) {
        outBytesWritten = 0;
        return false;
    }
    std::memcpy(outBuffer, edges.data, need);
    outBytesWritten = need;
    return true;
#else
    // Lightweight fallback: approximate edges via Sobel magnitude on a
    // CPU grayscale conversion. No hysteresis; threshold uses highThreshold.
    if (inputRgba.data == nullptr || inputRgba.width <= 0 || inputRgba.height <= 0 || inputRgba.channels < 3) {
        outBytesWritten = 0;
        return false;
    }

    const size_t need = requiredGrayBytes(inputRgba.width, inputRgba.height);
    if (outBuffer == nullptr || outBufferSize < need) {
        outBytesWritten = need;
        return false;
    }

    const int width = inputRgba.width;
    const int height = inputRgba.height;
    const int channels = inputRgba.channels;
    const int srcStride = (inputRgba.stride > 0) ? inputRgba.stride : width * channels;

    // 1) Convert to grayscale into a temporary buffer
    std::vector<uint8_t> gray(need);
    const uint8_t* srcRow = inputRgba.data;
    for (int y = 0; y < height; ++y) {
        const uint8_t* src = srcRow;
        uint8_t* dst = gray.data() + static_cast<size_t>(y) * static_cast<size_t>(width);
        for (int x = 0; x < width; ++x) {
            const int idx = x * channels;
            const uint8_t r = src[idx + 0];
            const uint8_t g = src[idx + 1];
            const uint8_t b = src[idx + 2];
            const int v = static_cast<int>(0.299f * r + 0.587f * g + 0.114f * b + 0.5f);
            dst[x] = static_cast<uint8_t>(std::clamp(v, 0, 255));
        }
        srcRow += srcStride;
    }

    // 2) Simple Sobel filtering (3x3) to approximate edges
    // Kernels:
    // Gx = [[-1, 0, 1], [-2, 0, 2], [-1, 0, 1]]
    // Gy = [[ 1, 2, 1], [ 0, 0, 0], [-1,-2,-1]]
    // We'll use |Gx| + |Gy| as magnitude approximation
    const int th = static_cast<int>(std::max(0.0, std::min(255.0, highThreshold))); // use highThreshold
    auto idxAt = [width](int xx, int yy) -> size_t { return static_cast<size_t>(yy) * static_cast<size_t>(width) + static_cast<size_t>(xx); };

    // Clear borders to 0
    std::fill(outBuffer, outBuffer + need, 0);
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            const int g00 = gray[idxAt(x - 1, y - 1)];
            const int g01 = gray[idxAt(x,     y - 1)];
            const int g02 = gray[idxAt(x + 1, y - 1)];
            const int g10 = gray[idxAt(x - 1, y    )];
            const int g11 = gray[idxAt(x,     y    )];
            const int g12 = gray[idxAt(x + 1, y    )];
            const int g20 = gray[idxAt(x - 1, y + 1)];
            const int g21 = gray[idxAt(x,     y + 1)];
            const int g22 = gray[idxAt(x + 1, y + 1)];

            // Sobel Gx, Gy
            const int gx = (-g00 + g02) + (-2 * g10 + 2 * g12) + (-g20 + g22);
            const int gy = ( g00 + 2 * g01 + g02) + (-g20 - 2 * g21 - g22);
            const int mag = std::abs(gx) + std::abs(gy); // L1 magnitude
            const int edge = (mag >> 3); // scale down to 0..~255 range
            outBuffer[idxAt(x, y)] = static_cast<uint8_t>((edge >= th) ? 255 : 0);
        }
    }

    outBytesWritten = need;
    return true;
#endif
}

} // namespace edgeviewer


