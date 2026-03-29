#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

 
class Shader {
public:
    // program iD
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();
    
    void use();
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    // if setting with 3 different values instead of just a vec3
    // use x y z in order so that it works
    template <typename... Args>
    void setVec3 (const std::string &name, const Args... args) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
};