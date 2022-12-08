#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

class Model3D {

public:

    float x, y, z; // position
    float rot_x, rot_y, rot_z; // rotation
    float scale_x, scale_y, scale_z; // scale
    float theta;
    bool has_normal_maps;

    glm::mat4 transformation_matrix;
    std::vector<GLuint> mesh_indices;
    std::vector<GLfloat> fullVertexData;

    Model3D(const char* path, float x, float y, float z,
        float rot_x, float rot_y, float rot_z,
        float scale_x, float scale_y, float scale_z, float theta, bool has_normal_maps);

    void init_data_regular();

    void init_data_with_normal_maps();

    void init_transformation_matrix();

    void rotate_on_axis(float rotateAngle, glm::vec3 rotateAxis);

    void transMatrix();

    void rotate(float rotateAngle, glm::vec3 rotateAxis);

    void move(glm::vec3 movePos);

    void move(glm::vec3 movePos, std::vector<Model3D> modelList);

    void scale(glm::vec3 scaleModel);

    void printDepth();

    bool checkCollision(glm::mat4 myPosition, glm::mat4 possibleCollisionPosition);

    void init_buffers(unsigned int VAO, unsigned int VBO);

    void init_buffers_with_normals(unsigned int VAO, unsigned int VBO);

    void draw(unsigned int transformationLoc, unsigned int startIndex, unsigned int size, unsigned int VAO);

};
