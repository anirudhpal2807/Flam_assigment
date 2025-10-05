#include "opencv_pipeline.hpp"

#include <algorithm>

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
    // Fallback to grayscale if OpenCV is not enabled in this build
    return processGrayscale(inputRgba, outBuffer, outBufferSize, outBytesWritten);
#endif
}

} // namespace edgeviewer


