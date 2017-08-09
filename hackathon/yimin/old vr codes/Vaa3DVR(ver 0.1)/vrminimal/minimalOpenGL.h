/**
  \file minimalOpenGL/minimalOpenGL.h
  \author Morgan McGuire, http://casual-effects.com
  Distributed with the G3D Innovation Engine http://casual-effects.com/g3d

  Minimal headers emulating a basic set of 3D graphics classes.
  This does not depend on any vector math library.

  This requires the headers and source (or static binary) from GLEW
   
     http://glew.sourceforge.net/

  and from GLFW 

     http://www.glfw.org/

  (All of which are distributed with G3D)

  All 3D math from http://graphicscodex.com
*/
#pragma once
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-braces"

#ifdef __APPLE__
#   define _OSX
#elif defined(_WIN64)
#   ifndef _WINDOWS
#       define _WINDOWS
#   endif
#elif defined(__linux__)
#   define _LINUX
#endif

#include <GL/glew.h>
#ifdef _WINDOWS
#   include <GL/wglew.h>
#elif defined(_LINUX)
#   include <GL/xglew.h>
#endif
//#include <GLFW/glfw3.h> 
#include <glfw3.h>


#ifdef _WINDOWS
    // Link against OpenGL
#   pragma comment(lib, "opengl32")
#   pragma comment(lib, "glew32")
#   pragma comment(lib, "glfw3")
#endif

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>
#include <vector>


void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    if ((type == GL_DEBUG_TYPE_ERROR) || (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)) {
        fprintf(stderr, "GL Debug: %s\n", message);
    }
}


GLFWwindow* initOpenGL(int width, int height, const std::string& title) {
    if (! glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW\n");
        ::exit(1);
    } 

    // Without these, shaders actually won't initialize properly
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

#   ifdef _DEBUG
       glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#   endif

    GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (! window) {
        fprintf(stderr, "ERROR: could not open window with GLFW\n");
        glfwTerminate();
        ::exit(2);
    }
    glfwMakeContextCurrent(window);

    // Start GLEW extension handler, with improved support for new features
    glewExperimental = GL_TRUE;
    glewInit();

    // Clear startup errors
    while (glGetError() != GL_NONE) {}

#   ifdef _DEBUG
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glEnable(GL_DEBUG_OUTPUT);
#       ifndef _OSX
            // Causes a segmentation fault on OS X
            glDebugMessageCallback(debugCallback, nullptr);
#       endif
#   endif

    // Negative numbers allow buffer swaps even if they are after the vertical retrace,
    // but that causes stuttering in VR mode
    glfwSwapInterval(0);

    fprintf(stderr, "GPU: %s (OpenGL version %s)\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));

    // Bind a single global vertex array (done this way since OpenGL 3)
    { GLuint vao; glGenVertexArrays(1, &vao); glBindVertexArray(vao); }

    // Check for errors
    { const GLenum error = glGetError(); assert(error == GL_NONE); }

    return window;
}


std::string loadTextFile(const std::string& filename) {
    std::stringstream buffer;
    buffer << std::ifstream(filename.c_str()).rdbuf();
    return buffer.str();
}


GLuint compileShaderStage(GLenum stage, const std::string& source) {
    GLuint shader = glCreateShader(stage);
    const char* srcArray[] = { source.c_str() };

    glShaderSource(shader, 1, srcArray, NULL);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE) {
        GLint logSize = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

        std::vector<GLchar> errorLog(logSize);
        glGetShaderInfoLog(shader, logSize, &logSize, &errorLog[0]);

        fprintf(stderr, "Error while compiling\n %s\n\nError: %s\n", source.c_str(), &errorLog[0]);
        assert(false);

        glDeleteShader(shader);
        shader = GL_NONE;
    }

    return shader;
}


GLuint createShaderProgram(const std::string& vertexShaderSource, const std::string& pixelShaderSource) {
    GLuint shader = glCreateProgram();

    glAttachShader(shader, compileShaderStage(GL_VERTEX_SHADER, vertexShaderSource));
    glAttachShader(shader, compileShaderStage(GL_FRAGMENT_SHADER, pixelShaderSource));
    glLinkProgram(shader);

    return shader;
}


/** Submits a full-screen quad at the far plane and runs a procedural sky shader on it.

    All matrices are 4x4 row-major
    \param light Light vector, must be normalized 
 */
void drawSky(int windowWidth, int windowHeight, float nearPlaneZ, float farPlaneZ, const float* cameraToWorldMatrix, const float* projectionMatrixInverse, const float* light) {
#   define VERTEX_SHADER(s) "#version 410\n" #s
#   define PIXEL_SHADER(s) VERTEX_SHADER(s)

    static const GLuint skyShader = createShaderProgram(VERTEX_SHADER
    (void main() {
        gl_Position = vec4(gl_VertexID & 1, gl_VertexID >> 1, 0.0, 0.5) * 4.0 - 1.0;
    }),

    PIXEL_SHADER
    (out vec3 pixelColor;

    uniform vec3  light;
    uniform vec2  resolution;
    uniform mat4  cameraToWorldMatrix;
    uniform mat4  invProjectionMatrix;

    float hash(vec2 p) { return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); }

    float noise(vec2 x) {
        vec2 i = floor(x);
        float a = hash(i);
        float b = hash(i + vec2(1.0, 0.0));
        float c = hash(i + vec2(0.0, 1.0));
        float d = hash(i + vec2(1.0, 1.0));

        vec2 f = fract(x);
        vec2 u = f * f * (3.0 - 2.0 * f);
        return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
    }

    float fbm(vec2 p) {
        const mat2 m2 = mat2(0.8, -0.6, 0.6, 0.8);
        float f = 0.5000 * noise(p); p = m2 * p * 2.02;
        f += 0.2500 * noise(p); p = m2 * p * 2.03;
        f += 0.1250 * noise(p); p = m2 * p * 2.01;
        f += 0.0625 * noise(p);
        return f / 0.9375;
    }

    vec3 render(in vec3 sun, in vec3 ro, in vec3 rd, in float resolution) {
        vec3 col;
        if (rd.y < 0.0) {
            // Ground
            float t = -ro.y / rd.y;
            vec2 P = ro.xz + t * rd.xz + vec2(0.5);
            vec2 Q = floor(P);
            // 1m^2 grid
            P = mod(P, 1.0);

            const float gridLineWidth = 0.1;
            float res = clamp(3000.0 / resolution, 1.0, 4.0);
            P = 1.0 - abs(P - 0.5) * 2.0;
            float d = clamp(min(P.x, P.y) / (gridLineWidth * clamp(t + res * 2.0, 1.0, 3.0)) + 0.5, 0.0, 1.0);
            float shade = mix(hash(100.0 + Q * 0.1) * 0.4, 0.3, min(t * t * 0.00001 / max(-rd.y, 0.001), 1.0)) + 0.6;
            col = vec3(pow(d, clamp(150.0 / (pow(max(t - 2.0, 0.1), res) + 1.0), 0.1, 15.0))) * shade + 0.1;
        } else {
            // Sky
            col = vec3(0.3, 0.55, 0.8) * (1.0 - 0.8 * rd.y) * 0.9;
            float sundot = clamp(dot(rd, sun), 0.0, 1.0);
            col += 0.25 * vec3(1.0, 0.7, 0.4) * pow(sundot, 8.0);
            col += 0.75 * vec3(1.0, 0.8, 0.5) * pow(sundot, 64.0);
            col = mix(col, vec3(1.0, 0.95, 1.0), 0.5 * smoothstep(0.5, 0.8, fbm((ro.xz + rd.xz * (250000.0 - ro.y) / rd.y) * 0.000008)));
        }
        return mix(col, vec3(0.7, 0.75, 0.8), pow(1.0 - max(abs(rd.y), 0.0), 8.0));
    }

    void main() {
        vec3 rd = normalize(mat3(cameraToWorldMatrix) * vec3((invProjectionMatrix * vec4(gl_FragCoord.xy / resolution.xy * 2.0 - 1.0, -1.0, 1.0)).xy, -1.0));
        pixelColor = render(light, cameraToWorldMatrix[3].xyz, rd, resolution.x);
    }));

    static const GLint lightUniform                      = glGetUniformLocation(skyShader, "light");
    static const GLint resolutionUniform                 = glGetUniformLocation(skyShader, "resolution");
    static const GLint cameraToWorldMatrixUniform        = glGetUniformLocation(skyShader, "cameraToWorldMatrix");
    static const GLint invProjectionMatrixUniform        = glGetUniformLocation(skyShader, "invProjectionMatrix");

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    glUseProgram(skyShader);
    glUniform3fv(lightUniform, 1, light);
    glUniform2f(resolutionUniform, float(windowWidth), float(windowHeight));
    glUniformMatrix4fv(cameraToWorldMatrixUniform, 1, GL_TRUE, cameraToWorldMatrix);
    glUniformMatrix4fv(invProjectionMatrixUniform, 1, GL_TRUE, projectionMatrixInverse);
    glDrawArrays(GL_TRIANGLES, 0, 3);

#   undef PIXEL_SHADER
#   undef VERTEX_SHADER
}


/*namespace Cube {
    const float position[][3] = { -.5f, .5f, -.5f, -.5f, .5f, .5f, .5f, .5f, .5f, .5f, .5f, -.5f, -.5f, .5f, -.5f, -.5f, -.5f, -.5f, -.5f, -.5f, .5f, -.5f, .5f, .5f, .5f, .5f, .5f, .5f, -.5f, .5f, .5f, -.5f, -.5f, .5f, .5f, -.5f, .5f, .5f, -.5f, .5f, -.5f, -.5f, -.5f, -.5f, -.5f, -.5f, .5f, -.5f, -.5f, .5f, .5f, -.5f, -.5f, .5f, .5f, -.5f, .5f, .5f, .5f, .5f, -.5f, -.5f, .5f, -.5f, -.5f, -.5f, .5f, -.5f, -.5f, .5f, -.5f, .5f };
    const float normal[][3]   = { 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f };
    const float tangent[][4]  = { 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, -1.f, 1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f };
    const float texCoord[][2] = { 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f };
    const int   index[]       = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11, 12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23 };
};//*/
namespace Cube {
	const float position[][3] = { -.5f, .5f, -.5f, -.5f, .5f, .5f, .5f, .5f, .5f, .5f, .5f, -.5f, -.5f, .5f, -.5f, -.5f, -.5f, -.5f, -.5f, -.5f, .5f, -.5f, .5f, .5f, .5f, .5f, .5f, .5f, -.5f, .5f, .5f, -.5f, -.5f, .5f, .5f, -.5f, .5f, .5f, -.5f, .5f, -.5f, -.5f, -.5f, -.5f, -.5f, -.5f, .5f, -.5f, -.5f, .5f, .5f, -.5f, -.5f, .5f, .5f, -.5f, .5f, .5f, .5f, .5f, -.5f, -.5f, .5f, -.5f, -.5f, -.5f, .5f, -.5f, -.5f, .5f, -.5f, .5f };
	const float normal[][3] = { 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f };
	const float tangent[][4] = { 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, -1.f, 1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f };
	const float texCoord[][2] = { 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f };
	const int   index[] = { 0, 1, 1, 2, 2, 3, 3, 0,    4, 5, 5, 6, 6, 7, 7, 4,         8,9,9,10,10,11,11,8,    12,13,13,14,14,15,15,12,   16,17,17,18,18,19,19,16,     20,21,21,22,22,23,23,20 };
};
/*namespace Lines {
    const float position[][3] = { -.5f, .5f, -.5f, -.5f, .5f, .5f, .5f, .5f, .5f, .5f, .5f, -.5f, -.5f, .5f, -.5f, -.5f, -.5f, -.5f, -.5f, -.5f, .5f, -.5f, .5f, .5f, .5f, .5f, .5f, .5f, -.5f, .5f, .5f, -.5f, -.5f, .5f, .5f, -.5f, .5f, .5f, -.5f, .5f, -.5f, -.5f, -.5f, -.5f, -.5f, -.5f, .5f, -.5f, -.5f, .5f, .5f, -.5f, -.5f, .5f, .5f, -.5f, .5f, .5f, .5f, .5f, -.5f, -.5f, .5f, -.5f, -.5f, -.5f, .5f, -.5f, -.5f, .5f, -.5f, .5f };
    const float normal[][3]   = { 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f };
    const float tangent[][4]  = { 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, -1.f, 1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f };
    const float texCoord[][2] = { 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f };
    const int   index[]       = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11, 12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23 };
};//*/


/** Loads a 24- or 32-bit BMP file into memory */
void loadBMP(const std::string& filename, int& width, int& height, int& channels, std::vector<unsigned char>& data) {
    std::fstream hFile(filename.c_str(), std::ios::in | std::ios::binary);
    if (! hFile.is_open()) { throw std::invalid_argument("Error: File Not Found."); }

    hFile.seekg(0, std::ios::end);
    size_t len = hFile.tellg();
    hFile.seekg(0, std::ios::beg);
    std::vector<unsigned char> header(len);
    hFile.read(reinterpret_cast<char*>(header.data()), 54);

    if ((header[0] != 'B') && (header[1] != 'M')) {
        hFile.close();
        throw std::invalid_argument("Error: File is not a BMP.");
    }

    if ((header[28] != 24) && (header[28] != 32)) {
        hFile.close();
        throw std::invalid_argument("Error: File is not uncompressed 24 or 32 bits per pixel.");
    }

    const short bitsPerPixel = header[28];
    channels = bitsPerPixel / 8;
    width  = header[18] + (header[19] << 8);
    height = header[22] + (header[23] << 8);
    unsigned int offset = header[10] + (header[11] << 8);
    unsigned int size = ((width * bitsPerPixel + 31) / 32) * 4 * height;
    data.resize(size);

    hFile.seekg(offset, std::ios::beg);
    hFile.read(reinterpret_cast<char*>(data.data()), size);
    hFile.close();

    // Flip the y axis
    std::vector<unsigned char> tmp;
    const size_t rowBytes = width * channels;
    tmp.resize(rowBytes);
    for (int i = height / 2 - 1; i >= 0; --i) {
        const int j = height - 1 - i;
        // Swap the rows
        memcpy(tmp.data(), &data[i * rowBytes], rowBytes);
        memcpy(&data[i * rowBytes], &data[j * rowBytes], rowBytes);
        memcpy(&data[j * rowBytes], tmp.data(), rowBytes);
    }

    // Convert BGR[A] format to RGB[A] format
    if (channels == 4) {
        // BGRA
        unsigned int* p = reinterpret_cast<unsigned int*>(data.data());
        for (int i = width * height - 1; i >= 0; --i) {
            const unsigned int x = p[i];
            p[i] = ((x >> 24) & 0xFF) | (((x >> 16) & 0xFF) << 8) | (((x >> 8) & 0xFF) << 16) | ((x & 0xFF) << 24);
        }
    } else {
        // BGR
        for (int i = (width * height - 1) * 3; i >= 0; i -= 3) {
            std::swap(data[i], data[i + 2]);
        }
    }
}

#pragma clang diagnostic pop
