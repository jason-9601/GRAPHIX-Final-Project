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

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);

/* Window size */
float screenWidth = 750.0f;
float screenHeight = 750.0f;

/* Camera */
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
bool firstMouse = true;
float lastX = 300, lastY = 300; // last mouse position (initalized to be in center of screen)

// camera
PerspectiveCamera pcam;
OrthoCamera ocam;
MyCamera camera(glm::vec3(0.0f, 0.0f, 10.f), glm::vec3(0.0f, 1.0f, 0.0f));

// for determining if perspective cam or ortho cam
glm::mat4 projection_matrix;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f; // time of last frame

bool isPers = true;
bool isOrtho = false;


void Key_Callback(GLFWwindow* window,
    int key,
    int scanCode,
    int action,
    int mods
)
{
    // press spacebar to switch control of main object and light source
    if (glfwGetKey(window, GLFW_KEY_1))
    {
        isPers = !isPers;
        isOrtho = false;
    }
    if (glfwGetKey(window, GLFW_KEY_2))
    {
        isOrtho = true;
        isPers = false;
    }
}

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
    const int textures_count = 2;
    const char* texture_filenames[textures_count] = { "3D/cat_texture.jpg", "3D/dolphin_texture.jpg"};

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
    glfwSetKeyCallback(window, Key_Callback);

    /* For mouse events */
    /* TODO: The view can be controlled by using the mouse */
    glfwSetCursorPosCallback(window, mouse_callback); 
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    /* Initialize shader object */
    Shader mainShader = Shader("Shaders/main.vert", "Shaders/main.frag");
    mainShader.useShaderProgram();

    /* Create object for testing */
    Model3D testObj = Model3D("3D/cat.obj", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.1f, 0.1f, 0.1f, 270.0f);
    testObj.rotate_on_axis(-90.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    Model3D dolphinObj = Model3D("3D/dolphin.obj", 0.0f, 5.0f, 5.0f, 0.0f, 0.0f, 1.0f, 0.05f, 0.05f, 0.05f, 90.0f);
    dolphinObj.rotate_on_axis(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    
    const int modelCount = 2;
    GLuint VAO[modelCount], VBO[modelCount];

    /* Setup VAO and VBOs */
    glGenVertexArrays(modelCount, VAO);
    glGenBuffers(modelCount, VBO);
    
    /* Draw Cat */
    glBindVertexArray(VAO[0]);
    testObj.init_buffers(VAO[0], VBO[0]);

    /* Draw Dolphin */
    glBindVertexArray(VAO[1]);
    dolphinObj.init_buffers(VAO[1], VBO[1]);

    glm::mat4 projection_matrix = pcam.GetPer();
    
    glm::mat4 viewMatrix;
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Get projection and view matrix from perspective camera */
        if (isPers || isOrtho) {
            viewMatrix = camera.GetViewMatrixThird();
        }
        else {
            viewMatrix = camera.GetViewMatrixFirst();
        }
        

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
        testObj.draw(transformationLoc, 0, testObj.fullVertexData.size() / 8, VAO[0]);

        glBindTexture(GL_TEXTURE_2D, textures[1]);
        glActiveTexture(GL_TEXTURE0);
        dolphinObj.draw(transformationLoc, 0, dolphinObj.fullVertexData.size() / 8, VAO[1]);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    // perspective view
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        projection_matrix = pcam.GetPer();
        
    }
    // orthographic view
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        projection_matrix = ocam.GetOrtho();
        const float nice = 90.f;
        camera.Pitch = nice;
        
    }

}

// for mouse movement
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}