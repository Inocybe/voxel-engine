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
    void setVec3 (const std::string &name, const Args... args)  const {
        if constexpr (sizeof...(args) == 1) {
            const auto& value = std::get<0>(std::forward_as_tuple(args...));
            glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
        } else if constexpr (sizeof...(args) == 3) {
            glUniform3f(glGetUniformLocation(ID, name.c_str()), args...);
        } else {
            static_assert(sizeof...(args) == 1 || sizeof...(args) == 3, "setVec3 requires either a glm::vec3 or three float arguments");
        }
    }
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
};