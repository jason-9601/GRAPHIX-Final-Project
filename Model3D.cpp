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
    float _scale_x, float _scale_y, float _scale_z, float _theta, bool has_normal_maps) {
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
    this->has_normal_maps = has_normal_maps;

    init_transformation_matrix();

    /* Load the object using tinyobj */
    bool success = tinyobj::LoadObj(
        &attributes,
        &shapes,
        &materials,
        &warning,
        &error,
        path);
 
    /* Initialize vertex data and indices depending on whether or not it will be normal mapped */
    if (has_normal_maps == false) {
        init_data_regular();
    } else {
        init_data_with_normal_maps();
    }

}

void Model3D::init_data_regular() {
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

void Model3D::init_data_with_normal_maps() {
    for (int i = 0; i < shapes[0].mesh.indices.size(); i++) {
        mesh_indices.push_back(shapes[0].mesh.indices[i].vertex_index);
    }

    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;

    for (int i = 0; i < shapes[0].mesh.indices.size(); i += 3) {

        tinyobj::index_t vData1 = shapes[0].mesh.indices[i];
        tinyobj::index_t vData2 = shapes[0].mesh.indices[i + 1];
        tinyobj::index_t vData3 = shapes[0].mesh.indices[i + 2];

        glm::vec3 v1 = glm::vec3(
            attributes.vertices[vData1.vertex_index * 3],
            attributes.vertices[(vData1.vertex_index * 3) + 1],
            attributes.vertices[(vData1.vertex_index * 3) + 2]);

        glm::vec3 v2 = glm::vec3(
            attributes.vertices[vData2.vertex_index * 3],
            attributes.vertices[(vData2.vertex_index * 3) + 1],
            attributes.vertices[(vData2.vertex_index * 3) + 2]);

        glm::vec3 v3 = glm::vec3(
            attributes.vertices[vData3.vertex_index * 3],
            attributes.vertices[(vData3.vertex_index * 3) + 1],
            attributes.vertices[(vData3.vertex_index * 3) + 2]);

        glm::vec2 uv1 = glm::vec2(
            attributes.texcoords[vData1.texcoord_index * 2],
            attributes.texcoords[(vData1.texcoord_index * 2) + 1]
        );

        glm::vec2 uv2 = glm::vec2(
            attributes.texcoords[vData2.texcoord_index * 2],
            attributes.texcoords[(vData2.texcoord_index * 2) + 1]
        );

        glm::vec2 uv3 = glm::vec2(
            attributes.texcoords[vData3.texcoord_index * 2],
            attributes.texcoords[(vData3.texcoord_index * 2) + 1]
        );

        glm::vec3 deltaPos1 = v2 - v1;
        glm::vec3 deltaPos2 = v3 - v1;

        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float r = 1.0f / ((deltaUV1.x * deltaUV2.y) - (deltaUV1.y * deltaUV2.x));

        glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
        glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

        tangents.push_back(tangent);
        tangents.push_back(tangent);
        tangents.push_back(tangent);

        bitangents.push_back(bitangent);
        bitangents.push_back(bitangent);
        bitangents.push_back(bitangent);

    }

    /* Populate vertex data */
    std::vector<GLfloat> fullVertexData;
    for (int i = 0; i < shapes[0].mesh.indices.size(); i++) {
        tinyobj::index_t vData = shapes[0].mesh.indices[i];

        /* X Y Z */
        this->fullVertexData.push_back(attributes.vertices[(vData.vertex_index * 3)]);
        this->fullVertexData.push_back(attributes.vertices[(vData.vertex_index * 3) + 1]);
        this->fullVertexData.push_back(attributes.vertices[(vData.vertex_index * 3) + 2]);

        /* Normals */
        this->fullVertexData.push_back(attributes.normals[(vData.normal_index * 3)]);
        this->fullVertexData.push_back(attributes.normals[(vData.normal_index * 3) + 1]);
        this->fullVertexData.push_back(attributes.normals[(vData.normal_index * 3) + 2]);

        /* U V */
        this->fullVertexData.push_back(attributes.texcoords[(vData.texcoord_index * 2)]);
        this->fullVertexData.push_back(attributes.texcoords[(vData.texcoord_index * 2) + 1]);

        this->fullVertexData.push_back(tangents[i].x);
        this->fullVertexData.push_back(tangents[i].y);
        this->fullVertexData.push_back(tangents[i].z);

        this->fullVertexData.push_back(bitangents[i].x);
        this->fullVertexData.push_back(bitangents[i].y);
        this->fullVertexData.push_back(bitangents[i].z);

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

/* Initialize buffers for obj with position, normals, and texture, tangents, bitangents */
void Model3D::init_buffers_with_normals(unsigned int VAO, unsigned int VBO) {
    /* Bind VBO */
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(GL_FLOAT) * this->fullVertexData.size(),
        this->fullVertexData.data(),
        GL_STATIC_DRAW);

    /* Position */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GL_FLOAT), (void*)0);
    glEnableVertexAttribArray(0);

    /* Normals */
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    /* Texture */
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GL_FLOAT), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    /* Tangents */
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GL_FLOAT), (void*)(8 * sizeof(GLfloat)));
    glEnableVertexAttribArray(3);

    /* Bitangents */
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GL_FLOAT), (void*)(11 * sizeof(GLfloat)));
    glEnableVertexAttribArray(4);
}

/* Pass in the uniform location for transformation as a parameter */
void Model3D::draw(unsigned int transformationLoc, unsigned int startIndex, unsigned int size, unsigned int VAO) {
    glUniformMatrix4fv(transformationLoc, 1, GL_FALSE, glm::value_ptr(this->transformation_matrix));
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, startIndex, size);
    glBindVertexArray(0);
}
