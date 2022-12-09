#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Player.h"

Player::Player(const char* path, float x, float y, float z,
    float rot_x, float rot_y, float rot_z,
    float scale_x, float scale_y, float scale_z, float theta, bool has_normal_maps, float box_offset)
    :Model3D(path, x, y, z,
        rot_x, rot_y,rot_z,
        scale_x, scale_y, scale_z,theta, has_normal_maps, box_offset) {

}
