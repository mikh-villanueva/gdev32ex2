/******************************************************************************
 * These are simple helper functions to facilitate file loading tasks
 * (particularly, text files, shaders, and textures).
 *
 * Note that if you will use this header file in a project with multiple .cpp
 * files, you should include it in only ONE .cpp file (due to the way the
 * stb_image library works).
 *
 * Happy hacking! - eric
 *****************************************************************************/

#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// loads an entire text file into a string
inline std::string gdevLoadFile(const char* filename)
{
    std::string result;
    std::ifstream file(filename);
    if (! file)
        std::cout << "Cannot read file '" << filename << "'\n";
    else
    {
        // read the file into an ostringstream
        // and convert the stream into a string
        std::ostringstream ss;
        ss << file.rdbuf();
        result = ss.str();
    }
    return result;
}

// compiles and links a GLSL shader program from the provided source files;
// returns the OpenGL object ID of the shader program (for use with glUseProgram)
inline GLuint gdevLoadShader(const char* vertexShaderFilename, const char* fragmentShaderFilename)
{
    std::string str;
    const char* cstr;

    // compile the vertex shader
    str = gdevLoadFile(vertexShaderFilename);
    if (str.empty())
        return 0;
    cstr = str.data();
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &cstr, NULL);
    glCompileShader(vertexShader);

    // compile the fragment shader
    str = gdevLoadFile(fragmentShaderFilename);
    if (str.empty())
    {
        glDeleteShader(vertexShader);
        return 0;
    }
    cstr = str.data();
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &cstr, NULL);
    glCompileShader(fragmentShader);

    // link the shaders into a single shader program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // for debugging, check if the shaders compiled correctly
    int success, length;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (! success)
    {
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &length);
        std::string err(length, ' ');
        glGetShaderInfoLog(vertexShader, length, NULL, err.data());
        std::cout << "Vertex shader file '" << vertexShaderFilename << "' compile error:\n" << err;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(program);
        return 0;
    }
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (! success)
    {
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &length);
        std::string err(length, ' ');
        glGetShaderInfoLog(fragmentShader, length, NULL, err.data());
        std::cout << "Fragment shader file '" << fragmentShaderFilename << "' compile error:\n" << err;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(program);
        return 0;
    }
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (! success)
    {
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        std::string err(length, ' ');
        glGetProgramInfoLog(program, length, NULL, err.data());
        std::cout << "Shader program link error:\n" << err;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(program);
        return 0;
    }

    // delete the shaders' compilation results (once they are linked, we don't need them anymore)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // return the final shader program
    return program;
}

// loads a texture with some common parameters (wrap mode, filtering, and mipmapping);
// returns the OpenGL object ID of the texture (for use with glBindTexture)
inline GLuint gdevLoadTexture(const char* textureFilename, int wrapMode, bool filter, bool generateMipmaps)
{
    // load the texture from a file
    int width, height, numChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(textureFilename, &width, &height, &numChannels, 0);
    if (! data)
    {
        std::cout << "Cannot read texture '" << textureFilename << "'\n";
        return 0;
    }

    // determine the texture's format
    int format;
    if (numChannels == 1)
        format = GL_RED;
    else if (numChannels == 2)
        format = GL_RG;
    else if (numChannels == 3)
        format = GL_RGB;
    else if (numChannels == 4)
        format = GL_RGBA;
    else
    {
        std::cout << "Texture '" << textureFilename << "' has an invalid format\n";
        stbi_image_free(data);
        return 0;
    }

    // generate the texture object
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set the texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter ? GL_LINEAR : GL_NEAREST);
    if (generateMipmaps)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        filter ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter ? GL_LINEAR : GL_NEAREST);
    }

    // upload the texture to the GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // needed for textures with less than 4 channels
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    if (generateMipmaps)
        glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    // return the final texture
    return texture;
}
