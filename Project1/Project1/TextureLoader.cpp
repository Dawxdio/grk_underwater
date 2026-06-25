#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "TextureLoader.h"
#include <iostream>

GLuint loadTexture(const char* filename)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data =
        stbi_load(filename, &width, &height,
            &channels, 0);

    if (!data)
    {
        std::cout << "Nie mozna zaladowac tekstury "
            << filename << std::endl;
        return 0;
    }

    GLenum format = GL_RGB;
    if (channels == 1) format = GL_RED;
    if (channels == 3) format = GL_RGB;
    if (channels == 4) format = GL_RGBA;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR);

    glTexParameteri(GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR);

    stbi_image_free(data);

    return tex;
}