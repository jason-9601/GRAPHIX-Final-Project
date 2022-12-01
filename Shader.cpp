#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Shader.h"

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath) {

    std::fstream vertSrc(vertexShaderPath);
    std::stringstream vertBuff;
    vertBuff << vertSrc.rdbuf();
    std::string vertString = vertBuff.str();
    const char* v = vertString.c_str();

    std::fstream fragSrc(fragmentShaderPath);
    std::stringstream fragBuff;
    fragBuff << fragSrc.rdbuf();
    std::string fragString = fragBuff.str();
    const char* f = fragString.c_str();

    /* Compile Vertex Shader */
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &v, NULL);
    glCompileShader(vertexShader);

    /* Check if vertex shader compiled successfully */
    checkCompilationStatus(vertexShader, "Vertex");

    /* Compile Fragment Shader */
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &f, NULL);
    glCompileShader(fragmentShader);

    /* Check if fragment shader compiled successfully */
    checkCompilationStatus(fragmentShader, "Fragment");

    /* Create and link shader program */
    this->shaderProgramID = glCreateProgram();
    glAttachShader(this->shaderProgramID, vertexShader);
    glAttachShader(this->shaderProgramID, fragmentShader);
    glLinkProgram(this->shaderProgramID);

}

void Shader::useShaderProgram() {
    glUseProgram(this->shaderProgramID);
}

void Shader::checkCompilationStatus(GLuint shader, const char* shaderType) {
    int  isSuccess;
    char infoLog[512];

    /* Check if compiled successfully */
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isSuccess);
    if (!isSuccess) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "Failed compilation of " << shaderType << " shader\n" << infoLog << std::endl;
    } else {
        std::cout << "Successfully compiled " << shaderType << " shader" << std::endl;
    }
}

GLuint Shader::getID() {
    return this->shaderProgramID;
}
