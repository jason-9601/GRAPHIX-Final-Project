#pragma once
#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<string>
#include<iostream>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<vector>

class Light
{
public:
    glm::vec3 lightColor; // light color
    float constant; // light constant
    float linear; // light linear
    float quadratic; // light quadratic

    float ambientStr; // ambient strength
    glm::vec3 ambientColor; // ambient color

    float specStr; // specular strength
    float specPhong; // specular phong strength

    // function for setting up and updating light data
    void LightData(glm::vec3 lightColor, float constant, float linear, float quadratic, float ambientStr, glm::vec3 ambientColor, float specStr, float specPhong)
    {
        this->lightColor = lightColor;
        this->constant = constant;
        this->linear = linear;
        this->quadratic = quadratic;
        this->ambientStr = ambientStr;
        this->ambientColor = ambientColor;
        this->specStr = specStr;
        this->specPhong = specPhong;
    }

};

class DirectionalLight : public Light
{
public:
    glm::vec3 direction; // directional light direction
    // directional light constructor
    DirectionalLight(glm::vec3 direction)
    {
        this->direction = direction;
    }

    void setDirLight(glm::vec3 direction)
    {
        this->direction = direction;
    }
};

class PointLight : public Light
{
public:
    glm::vec3 lightPos; // light position
    // point light constructor
    PointLight(glm::vec3 lightPos)
    {
        this->lightPos = lightPos;
    }

    void setPointLight(glm::vec3 lightPos)
    {
        this->lightPos = lightPos;
    }
};
