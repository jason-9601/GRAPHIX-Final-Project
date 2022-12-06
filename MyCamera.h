#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

/* Default camera values */
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SENSITIVITY = 0.1f;

class MyCamera {

    public:
    // camera attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // for euler angles
    float Yaw;
    float Pitch;
    // for mouse movement
    float MouseSensitivity;

    // constructor with vectors
    MyCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MouseSensitivity(SENSITIVITY)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    MyCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MouseSensitivity(SENSITIVITY)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix which only looks at or rotates at main object 
    glm::mat4 GetViewMatrixThird()
    {
        const float radius = 10.0f;
        float camX = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch)) * radius;
        float camY = sin(glm::radians(Pitch)) * radius;
        float camZ = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch)) * radius;

        return glm::lookAt(glm::vec3(camX, camY, camZ), Front, Up);
    }

    glm::mat4 GetViewMatrixFirst()
    {
        updateCameraVectors();
        return glm::lookAt(Position, Position + (Front * glm::vec3(-1.f, -1.f, -1.f)), Up);
    }

    // processes mouse movement from main cpp
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors();
    }

    private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

class OrthoCamera : public MyCamera {
    public:
        /* Orthographic view function */
        glm::mat4 GetOrtho() {
            return glm::ortho(-15.f, 15.f,
                -15.f, 15.f,
                -1000.f, 1000.f);
        }
};

class PerspectiveCamera : public MyCamera {
    public:
        /* Perspective view function */
        glm::mat4 GetPer(float rad) {
            return glm::perspective(
                glm::radians(rad), // FOV
                750.f / 750.f, // Aspect Ratio
                0.1f, // Near
                100.0f // Far
            );
        }
};

