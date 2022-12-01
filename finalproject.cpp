#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <string>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Model3D.h"
#include "MyCamera.h"

/* Window size */
float screenWidth = 750.0f;
float screenHeight = 750.0f;

/* Camera */
PerspectiveCamera perspectiveCamera;

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(screenWidth, screenHeight, "Final Project", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    gladLoadGL();

    /* Variables for texture initialization */
    const int textures_count = 1;
    const char* texture_filenames[textures_count] = { "3D/cat_texture.jpg" };

    int img_width, img_height, color_channels;

    GLuint textures[textures_count];
    glGenTextures(textures_count, textures);

    /* Load the respective textures */
    for (int i = 0; i < textures_count; i++) {
        stbi_set_flip_vertically_on_load(true);

        unsigned char* tex_bytes = stbi_load(texture_filenames[i], &img_width, &img_height, &color_channels, 0);

        glBindTexture(GL_TEXTURE_2D, textures[i]);

        std::string current_filename = texture_filenames[i];

        /* Check if texture is a png or jpg then load with respective parameters */
        if (current_filename.find("png") != std::string::npos) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_bytes);
        }
        else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_bytes);
        }

        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(tex_bytes);

        glEnable(GL_DEPTH_TEST);
    }

    /* For keyboard events */
    /* TODO: The Player ship can be controlled using WASDQE */
    //glfwSetKeyCallback(window, Key_Callback);

    /* For mouse events */
    /* TODO: The view can be controlled by using the mouse */
    //glfwSetCursorPosCallback(window, Mouse_Callback);

    /* Initialize shader object */
    Shader mainShader = Shader("Shaders/main.vert", "Shaders/main.frag");
    mainShader.useShaderProgram();

    /* Create object for testing */
    Model3D testObj = Model3D("3D/cat.obj", 0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 1.0f, 0.1f, 0.1f, 0.1f, 270.0f);
    testObj.rotate_on_axis(-90.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    GLuint VAO, VBO;

    /* Setup VAO for test object */
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    /* Bind VBO */
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(GL_FLOAT) * testObj.fullVertexData.size(),
        testObj.fullVertexData.data(),
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

    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Get projection and view matrix from perspective camera */
        glm::mat4 projection_matrix = perspectiveCamera.GetPer();
        glm::mat4 viewMatrix = perspectiveCamera.GetViewMatrix();

        /* Set uniforms in shaders */
        unsigned int projectionLoc = glGetUniformLocation(mainShader.getID(), "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

        unsigned int viewLoc = glGetUniformLocation(mainShader.getID(), "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
        
        unsigned int transformationLoc = glGetUniformLocation(mainShader.getID(), "transform");

        GLuint texOAddress = glGetUniformLocation(mainShader.getID(), "tex0");
        glUniform1i(texOAddress, 0);

        /* Draw object with texture */
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glActiveTexture(GL_TEXTURE0);
        testObj.draw(transformationLoc, 0, testObj.fullVertexData.size() / 8, VAO);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}
