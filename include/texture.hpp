#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Texture {
public:
    unsigned int texture;
    unsigned int unit;
    GLfloat maxAnisotropy;
    GLfloat desiredAnisotropy = 8.0f;

    // Calling this function is intuitive
    Texture(const char* texturePath, const unsigned int textureUnit);
    ~Texture();

    // This needs to be called for spcific shader after it is set to be used
    // The textureUnit set in the creation is the index this texture gets binded to
    // If this isn't called, there is no texture
    void bind() const;
};
