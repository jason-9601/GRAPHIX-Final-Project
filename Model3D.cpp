#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "Model3D.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

std::vector<tinyobj::shape_t> shapes;
std::vector<tinyobj::material_t> materials;
std::string warning, error;
tinyobj::attrib_t attributes;

Model3D::Model3D(const char* path, float _x, float _y, float _z,
    float _rot_x, float _rot_y, float _rot_z,
    float _scale_x, float _scale_y, float _scale_z, float _theta) {
    this->x = _x;
    this->y = _y;
    this->z = _z;
    this->rot_x = _rot_x;
    this->rot_y = _rot_y;
    this->rot_z = _rot_z;
    this->scale_x = _scale_x;
    this->scale_y = _scale_y;
    this->scale_z = _scale_z;
    this->theta = _theta;

    init_transformation_matrix();

    /* Load the object using tinyobj */
    bool success = tinyobj::LoadObj(
        &attributes,
        &shapes,
        &materials,
        &warning,
        &error,
        path);
 
    /* Iterate over the multiple shapes of obj */
     for (int s = 0; s < shapes.size(); s++) {
         for (int i = 0; i < shapes[s].mesh.indices.size(); i++) {
             this->mesh_indices.push_back(shapes[s].mesh.indices[i].vertex_index);
         }
     }

    /* Populate vertex data for main obj */
    for (int s = 0; s < shapes.size(); s++) {
        for (int i = 0; i < shapes[s].mesh.indices.size(); i++) {
            tinyobj::index_t vData = shapes[s].mesh.indices[i];

            // X Y Z //
            this->fullVertexData.push_back(attributes.vertices[(vData.vertex_index * 3)]);
            this->fullVertexData.push_back(attributes.vertices[(vData.vertex_index * 3) + 1]);
            this->fullVertexData.push_back(attributes.vertices[(vData.vertex_index * 3) + 2]);

            // Normals //
            this->fullVertexData.push_back(attributes.normals[(vData.normal_index * 3)]);
            this->fullVertexData.push_back(attributes.normals[(vData.normal_index * 3) + 1]);
            this->fullVertexData.push_back(attributes.normals[(vData.normal_index * 3) + 2]);

            // U V //
            this->fullVertexData.push_back(attributes.texcoords[(vData.texcoord_index * 2)]);
            this->fullVertexData.push_back(attributes.texcoords[(vData.texcoord_index * 2) + 1]);
        }
    }

}

void Model3D::init_transformation_matrix() {
    this->transformation_matrix = glm::mat4(1.0f);

    this->transformation_matrix = glm::translate(this->transformation_matrix,
        glm::vec3(this->x, this->y, this->z));

    this->transformation_matrix = glm::scale(this->transformation_matrix,
        glm::vec3(this->scale_x, this->scale_y, this->scale_z));

    this->transformation_matrix = glm::rotate(this->transformation_matrix,
        glm::radians(this->theta),
        glm::normalize(glm::vec3(this->rot_x, this->rot_y, this->rot_z)));
}

void Model3D::rotate_on_axis(float rotateAngle, glm::vec3 rotateAxis) {
    this->transformation_matrix = glm::rotate(this->transformation_matrix,
        rotateAngle,
        rotateAxis);
}

/* Initialize buffers for obj with position, normals, and texture */
void Model3D::init_buffers(unsigned int VAO, unsigned int VBO) {
    /* Bind VBO */
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(GL_FLOAT) * this->fullVertexData.size(),
        this->fullVertexData.data(),
        GL_STATIC_DRAW);

    /* Position */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)0);
    glEnableVertexAttribArray(0);

    /* Normals */
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    /* Texture */
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
}

/* Pass in the uniform location for transformation as a parameter */
void Model3D::draw(unsigned int transformationLoc, unsigned int startIndex, unsigned int size, unsigned int VAO) {
    glUniformMatrix4fv(transformationLoc, 1, GL_FALSE, glm::value_ptr(this->transformation_matrix));
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, startIndex, size);
    glBindVertexArray(0);
}
