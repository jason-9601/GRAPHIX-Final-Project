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
#include "Light.h"

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

// lights (pointlight and directional)
PointLight plight(glm::vec3(0, 0, 5));
DirectionalLight dlight(glm::vec3(4.f, 11.f, -3.f));

// point light & directional light intensity
float plight_str = .05f;
float dlight_str = .3f;


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
    const int textures_count = 7;
    const char* texture_filenames[textures_count] = { "3D/cat_texture.jpg", "3D/dolphin_texture.jpg",
    "3D/shark_texture.jpg", "3D/turtle_texture.jpg", "3D/angelfish_texture.jpg", "3D/coral_texture.jpg", 
    "3D/diver_texture.jpg" };

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

    std::vector<Model3D> modelList;

    /* Create object for testing */
    Model3D testObj = Model3D("3D/cat.obj", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.1f, 0.1f, 0.1f, 270.0f);
    testObj.rotate_on_axis(-90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    modelList.push_back(testObj);

    /* https://free3d.com/3d-model/-dolphin-v1--12175.html */
    Model3D dolphinObj = Model3D("3D/dolphin.obj", 0.0f, 5.0f, 5.0f, 0.0f, 0.0f, 1.0f, 0.05f, 0.05f, 0.05f, 90.0f);
    dolphinObj.rotate_on_axis(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    modelList.push_back(dolphinObj);

    /* https://free3d.com/3d-model/shark-v2--367955.html */
    Model3D sharkObj = Model3D("3D/shark.obj", 0.0f, 5.0f, 10.0f, 0.0f, 0.0f, 1.0f, 0.05f, 0.05f, 0.05f, 90.0f);
    sharkObj.rotate_on_axis(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    modelList.push_back(sharkObj);

    /* https://free3d.com/3d-model/-sea-turtle-v1--427786.html */
    Model3D turtleObj = Model3D("3D/turtle.obj", 0.0f, -5.0f, 5.0f, 0.0f, 0.0f, 1.0f, 0.1f, 0.1f, 0.1f, 90.0f);
    turtleObj.rotate_on_axis(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    modelList.push_back(turtleObj);

    /* https://free3d.com/3d-model/coral-beauty-angelfish-v1--473554.html */
    Model3D angelfishObj = Model3D("3D/angelfish.obj", -5.0f, -7.0f, 0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 2.0f, 2.0f, 90.0f);
    modelList.push_back(angelfishObj);

    /* https://free3d.com/3d-model/coral-v1--901825.html */
    Model3D coralObj = Model3D("3D/coral.obj", 8.0f, -5.0f, 5.0f, 0.0f, 0.0f, 1.0f, 0.1f, 0.1f, 0.1f, 90.0f);
    modelList.push_back(coralObj);

    /* https://free3d.com/3d-model/aquarium-deep-sea-diver-v1--436500.html */
    Model3D diverObj = Model3D("3D/diver.obj", 5.0f, -5.0f, -5.0f, 0.0f, 0.0f, 1.0f, 0.3f, 0.3f, 0.3f, 90.0f);
    modelList.push_back(diverObj);

    const int modelCount = 7;
    GLuint VAO[modelCount], VBO[modelCount];

    /* Setup VAO and VBOs */
    glGenVertexArrays(modelCount, VAO);
    glGenBuffers(modelCount, VBO);

    /* Inititalize buffers for each obj in list */
    for (int i = 0; i < modelCount; i++) {
        glBindVertexArray(VAO[i]);
        modelList[i].init_buffers(VAO[i], VBO[i]);
    }

    glm::mat4 projection_matrix = pcam.GetPer();
    
    glm::mat4 viewMatrix;

    float constant = .01f; // light constant
    float linear = 0.009f; // light linear
    float quadratic = 0.0032f; // light quadratic

    glm::vec3 ambientColor = glm::vec3(1, 1, 1);

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

        // setting point light data
        plight.LightData(glm::vec3(plight_str, plight_str, plight_str), constant, linear, quadratic, .01f, glm::vec3(1, 1, 1), .01f, .5f);
        // setting directional light data
        dlight.LightData(glm::vec3(dlight_str, dlight_str, dlight_str), 0, 0, 0, .5f, ambientColor, .01f, .5f);
        
        // point light data to transfer to shader program
        unsigned int lightPosLoc = glGetUniformLocation(mainShader.getID(), "lightPos");
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(plight.lightPos));

        unsigned int colorLoc = glGetUniformLocation(mainShader.getID(), "ourColor");
        glUniform3fv(colorLoc, 1, glm::value_ptr(glm::vec3(1,1,1)));

        unsigned int lightColorLoc = glGetUniformLocation(mainShader.getID(), "lightColor");
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(plight.lightColor));

        unsigned int constantLoc = glGetUniformLocation(mainShader.getID(), "constant");
        glUniform1f(constantLoc, plight.constant);

        unsigned int linearLoc = glGetUniformLocation(mainShader.getID(), "linear");
        glUniform1f(linearLoc, plight.linear);

        unsigned int quadraticLoc = glGetUniformLocation(mainShader.getID(), "quadratic");
        glUniform1f(quadraticLoc, plight.quadratic);

        unsigned int ambientStrLoc = glGetUniformLocation(mainShader.getID(), "ambientStr");
        glUniform1f(ambientStrLoc, plight.ambientStr);

        unsigned int ambientColorLoc = glGetUniformLocation(mainShader.getID(), "ambientColor");
        glUniform3fv(ambientColorLoc, 1, glm::value_ptr(plight.ambientColor));

        unsigned int cameraPosLoc = glGetUniformLocation(mainShader.getID(), "cameraPos");
        glUniform3fv(cameraPosLoc, 1, glm::value_ptr(camera.Position));

        unsigned int specStrLoc = glGetUniformLocation(mainShader.getID(), "specStr");
        glUniform1f(specStrLoc, plight.specStr);

        unsigned int specPhogLoc = glGetUniformLocation(mainShader.getID(), "specPhong");
        glUniform1f(specPhogLoc, plight.specPhong);

        // directional light data to transfer to shader program

        unsigned int dirlightPosLoc = glGetUniformLocation(mainShader.getID(), "direction");
        glUniform3fv(dirlightPosLoc, 1, glm::value_ptr(dlight.direction));

        unsigned int dirlightColorLoc = glGetUniformLocation(mainShader.getID(), "dirlightColor");
        glUniform3fv(dirlightColorLoc, 1, glm::value_ptr(dlight.lightColor));

        unsigned int dirambientStrLoc = glGetUniformLocation(mainShader.getID(), "dirambientStr");
        glUniform1f(dirambientStrLoc, dlight.ambientStr);

        unsigned int dirambientColorLoc = glGetUniformLocation(mainShader.getID(), "dirambientColor");
        glUniform3fv(dirambientColorLoc, 1, glm::value_ptr(dlight.ambientColor));

        unsigned int dirspecStrLoc = glGetUniformLocation(mainShader.getID(), "dirspecStr");
        glUniform1f(dirspecStrLoc, dlight.specStr);

        unsigned int dirspecPhogLoc = glGetUniformLocation(mainShader.getID(), "dirspecPhong");
        glUniform1f(dirspecPhogLoc, dlight.specPhong);

        /* Set uniforms in shaders */
        unsigned int projectionLoc = glGetUniformLocation(mainShader.getID(), "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

        unsigned int viewLoc = glGetUniformLocation(mainShader.getID(), "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
        
        unsigned int transformationLoc = glGetUniformLocation(mainShader.getID(), "transform");

        GLuint texOAddress = glGetUniformLocation(mainShader.getID(), "tex0");
        glUniform1i(texOAddress, 0);

        /* Draw submarine object / temporarily still a cat */
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glActiveTexture(GL_TEXTURE0);
        modelList[0].draw(transformationLoc, 0, testObj.fullVertexData.size() / 8, VAO[0]);

        /* Draw rest of models in dolphin, shark, turtle, angelfish, coral, diver */
        for (int i = 1; i < modelList.size(); i++) {
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glActiveTexture(GL_TEXTURE0);
            modelList[i].draw(transformationLoc, 0, modelList[i].fullVertexData.size() / 8, VAO[i]);
        }
  
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