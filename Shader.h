#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {

private:

    GLuint shaderProgramID;

public:

    Shader(const char* vertexShaderPath, const char* fragmentShaderPath);

    void useShaderProgram();

    void checkCompilationStatus(GLuint shader, const char* shaderType);

    /* TODO: functions for setting vectors */

};
