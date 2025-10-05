#include "gl_renderer.hpp"

#include <cstring>

namespace edgegl {

namespace {
    // Simple textured quad
    static const GLfloat kVertices[] = {
        // x, y,    u, v
        -1.0f, -1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 0.0f,
    };

    static const GLushort kIndices[] = { 0, 1, 2, 0, 2, 3 };

    static const char* kVS =
        "attribute vec2 aPos;\n"
        "attribute vec2 aUV;\n"
        "varying vec2 vUV;\n"
        "void main(){\n"
        "  vUV = aUV;\n"
        "  gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "}";

    static const char* kFS =
        "precision mediump float;\n"
        "varying vec2 vUV;\n"
        "uniform sampler2D uTex;\n"
        "void main(){\n"
        "  float g = texture2D(uTex, vUV).r;\n"
        "  gl_FragColor = vec4(g, g, g, 1.0);\n"
        "}";
}

GLRenderer::GLRenderer() = default;
GLRenderer::~GLRenderer() {
    if (texture_) glDeleteTextures(1, &texture_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (ibo_) glDeleteBuffers(1, &ibo_);
    if (program_) glDeleteProgram(program_);
}

GLuint GLRenderer::compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) { glDeleteShader(s); return 0; }
    return s;
}

bool GLRenderer::createProgram() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, kVS);
    if (!vs) return false;
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, kFS);
    if (!fs) { glDeleteShader(vs); return false; }

    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glBindAttribLocation(program_, 0, "aPos");
    glBindAttribLocation(program_, 1, "aUV");
    glLinkProgram(program_);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint ok = 0; glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (!ok) { glDeleteProgram(program_); program_ = 0; return false; }

    attrPos_ = 0;
    attrUV_ = 1;
    uniSampler_ = glGetUniformLocation(program_, "uTex");
    return true;
}

bool GLRenderer::initialize() {
    if (!createProgram()) return false;

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), kVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return true;
}

void GLRenderer::resize(int width, int height) {
    viewportW_ = width;
    viewportH_ = height;
}

bool GLRenderer::uploadGrayTexture(const uint8_t* data, int width, int height) {
    if (!data || width <= 0 || height <= 0) return false;
    glBindTexture(GL_TEXTURE_2D, texture_);
    // Using GL_LUMINANCE for ES 2.0 grayscale compatibility
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
    return true;
}

void GLRenderer::renderFrame() {
    glViewport(0, 0, viewportW_, viewportH_);
    glClearColor(0.1f, 0.12f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glUniform1i(uniSampler_, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glEnableVertexAttribArray(attrPos_);
    glEnableVertexAttribArray(attrUV_);
    glVertexAttribPointer(attrPos_, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (const GLvoid*)0);
    glVertexAttribPointer(attrUV_, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (const GLvoid*)(sizeof(GLfloat) * 2));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(attrPos_);
    glDisableVertexAttribArray(attrUV_);
}

} // namespace edgegl


