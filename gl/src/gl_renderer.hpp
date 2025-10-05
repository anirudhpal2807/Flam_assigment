#pragma once

#include <GLES2/gl2.h>
#include <cstdint>

namespace edgegl {

class GLRenderer {
public:
    GLRenderer();
    ~GLRenderer();

    bool initialize();
    void resize(int width, int height);
    void renderFrame();

    // Upload a single-channel (grayscale) image as LUMINANCE texture
    bool uploadGrayTexture(const uint8_t* data, int width, int height);

private:
    GLuint program_ = 0;
    GLuint vbo_ = 0;
    GLuint ibo_ = 0;
    GLuint texture_ = 0;
    GLint attrPos_ = -1;
    GLint attrUV_ = -1;
    GLint uniSampler_ = -1;
    int viewportW_ = 0;
    int viewportH_ = 0;

    bool createProgram();
    static GLuint compileShader(GLenum type, const char* src);
};

} // namespace edgegl


