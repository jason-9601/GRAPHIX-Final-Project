#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Model3D.h"

class Player : public Model3D {

public:

    Player(const char* path, float x, float y, float z,
        float rot_x, float rot_y, float rot_z,
        float scale_x, float scale_y, float scale_z, float theta, bool has_normal_maps, float box_offset);

};
