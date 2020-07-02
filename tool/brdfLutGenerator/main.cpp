#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <vector>
#define LUT_SIZE 512
#ifndef SHADER_DIR
#   define SHADER_DIR ""
#endif
using namespace std;

#define OFILE_NAME "brdf.bin"

void writePng(const float* data)
{
    constexpr int size = 3 * LUT_SIZE * LUT_SIZE;
    vector<unsigned char> buffer(size);
    for (int i = 0; i < size; ++i)
    {
        buffer[i] = static_cast<unsigned char>(255.999f * data[i]);
    }

    std::cout << "writing png\n";
    stbi_write_png("brdf.png", LUT_SIZE, LUT_SIZE, 3, buffer.data(), 3 * LUT_SIZE);
}

void readBin(float* in)
{
    ifstream bin(OFILE_NAME, ios::out | ios::binary);
    if (!bin.is_open())
        throw runtime_error("Failed to open " OFILE_NAME " for read");

    constexpr int size = 2 * LUT_SIZE * LUT_SIZE;
    bin.read(reinterpret_cast<char*>(in), size * sizeof(float));

    vector<unsigned char> buffer(3 * LUT_SIZE * LUT_SIZE);
    for (int i = 0; i < LUT_SIZE * LUT_SIZE; ++i)
    {
        buffer[3 * i] = static_cast<unsigned char>(255.999f * in[2 * i]);
        buffer[3 * i + 1] = static_cast<unsigned char>(255.999f * in[2 * i + 1]);
        buffer[3 * i + 2] = 0;
    }

    std::cout << "writing png\n";
    stbi_write_png("brdf.png", LUT_SIZE, LUT_SIZE, 3, buffer.data(), 3 * LUT_SIZE);
}

void writeBin(const float* data)
{
    ofstream bin(OFILE_NAME, ios::out | ios::binary);
    if (!bin.is_open())
        throw runtime_error("Failed to open " OFILE_NAME " for write");

    int size = 2 * LUT_SIZE * LUT_SIZE;

    vector<float> buffer(size);
    for (int i = 0; i < LUT_SIZE * LUT_SIZE; ++i)
    {
        buffer[2 * i] = data[3 * i];
        buffer[2 * i + 1] = data[3 * i + 1];
    }

    bin.write((char*) buffer.data(), size * sizeof(float));
    bin.close();

    if (!bin.good())
        throw runtime_error("Error occured when writing to " OFILE_NAME);
}

string readAscciFile(const char* path)
{
    ifstream f(path);
    if (!f.good())
        throw runtime_error("Failed to openfile: " + string(path));

    return string((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
}

GLuint createShaderFromString(const string& source, GLenum type)
{
    const int MAX_LOG_SIZE = 512;
    GLuint handle = glCreateShader(type);
    const char* sources[] = { source.c_str() };
    glShaderSource(handle, 1, sources, NULL);
    glCompileShader(handle);
    int success;
    char log[MAX_LOG_SIZE];
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(handle, MAX_LOG_SIZE, NULL, log);
        log[MAX_LOG_SIZE - 1] = '\0'; // prevent overflow
        string error("glsl: Failed to compile shader\n");
        error.append(log).pop_back(); // remove new line
        throw runtime_error(error);
    }

    return handle;
}

GLuint compileShader(const char* vert, const char* frag)
{
    const string vertSource = readAscciFile(vert);
    GLuint vertShader = createShaderFromString(vertSource, GL_VERTEX_SHADER);
    const string fragSource = readAscciFile(frag);
    GLuint fragShader = createShaderFromString(fragSource, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    const int MAX_LOG_SIZE = 512;
    int success;
    char log[MAX_LOG_SIZE];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, MAX_LOG_SIZE, NULL, log);
        log[MAX_LOG_SIZE - 1] = '\0'; // prevent overflow
        string error("glsl: Failed to link program\n");
        error.append(log).pop_back(); // remove new line
        throw runtime_error(error);
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    return program;
}

struct Vertex
{
    GLfloat x, y;
};

Vertex vertices[] = {
    { -1.0f, +1.0f },
    { -1.0f, -1.0f },
    { +1.0f, +1.0f },
    { +1.0f, +1.0f },
    { -1.0f, -1.0f },
    { +1.0f, -1.0f },
};

int main()
{
    glfwSetErrorCallback([](int error, const char* desc)
    {
        throw runtime_error("[Error] glfw: " + string(desc));
    });

    try
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        GLFWwindow* window = glfwCreateWindow(LUT_SIZE, LUT_SIZE, "BRDF LUT", 0, 0);
        glfwMakeContextCurrent(window);
        if (gladLoadGL() == 0)
            throw runtime_error("[Error] glad: failed to load gl functions");

        // shader
        GLuint brdfProgram = compileShader(SHADER_DIR "quad.vert", SHADER_DIR "brdf.frag");
        GLuint quadProgram = compileShader(SHADER_DIR "quad.vert", SHADER_DIR "visualization.frag");
        // vao
        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glEnableVertexAttribArray(0);
        glUseProgram(brdfProgram);

        // texture
        // unsigned int brdfLUTTexture;
        // glGenTextures(1, &brdfLUTTexture);
        // glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, LUT_SIZE, LUT_SIZE, 0, GL_RGB, GL_FLOAT, 0);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // // fbo
        // unsigned int captureFBO;
        // unsigned int captureRBO;
        // glGenFramebuffers(1, &captureFBO);
        // glGenRenderbuffers(1, &captureRBO);
        // glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        // glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

        // glViewport(0, 0, LUT_SIZE, LUT_SIZE);
        // glUseProgram(brdfProgram);
        // glClear(GL_COLOR_BUFFER_BIT);
        // glDrawArrays(GL_TRIANGLES, 0, 6);

        // // read pixels
        // vector<float> buffer(3 * LUT_SIZE * LUT_SIZE);
        // // float* buffer = new float[3 * LUT_SIZE * LUT_SIZE];
        // glReadPixels(0, 0, LUT_SIZE, LUT_SIZE, GL_RGB, GL_FLOAT, buffer.data());
        // writeBin(buffer);
        // writePng(buffer.data());
        float* in = new float[2 * LUT_SIZE * LUT_SIZE];
        readBin(in);

        unsigned int brdfLUTTexture;
        glGenTextures(1, &brdfLUTTexture);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, LUT_SIZE, LUT_SIZE, 0, GL_RG, GL_FLOAT, in);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        delete [] in;

        // render to screen
        glUseProgram(quadProgram);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        while(!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            glViewport(0, 0, LUT_SIZE, LUT_SIZE);
            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glfwSwapBuffers(window);
        }

        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(brdfProgram);

        glfwDestroyWindow(window);
        glfwTerminate();
    }
    catch(const runtime_error& e)
    {
        cerr << e.what() << endl;
        return -1;
    }

    return 0;
}
