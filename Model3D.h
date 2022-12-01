#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

class Model3D {

public:

    float x, y, z; // position
    float rot_x, rot_y, rot_z; // rotation
    float scale_x, scale_y, scale_z; // scale
    float theta;

    glm::mat4 transformation_matrix;
    std::vector<GLuint> mesh_indices;
    std::vector<GLfloat> fullVertexData;

    Model3D(const char* path, float x, float y, float z,
        float rot_x, float rot_y, float rot_z,
        float scale_x, float scale_y, float scale_z, float theta);

    void init_transformation_matrix();

    void rotate_on_axis(float rotateAngle, glm::vec3 rotateAxis);

    void draw(unsigned int transformationLoc, unsigned int startIndex, unsigned int size, unsigned int VAO);

};