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
MyCamera camera(glm::vec3(0.0f, 5.0f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f));

// for determining if perspective cam or ortho cam
glm::mat4 projection_matrix;
glm::mat4 skybox_projection_matrix;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f; // time of last frame

bool isPers = true;
bool isOrtho = false;

// mainObj initialization, rot, scale, and pos
float x_mod, z_mod = 0.0f;
float y_mod = 0.005;
float theta = 0.0f;

// lights (pointlight and directional)
PointLight plight(glm::vec3(0, 0, 5));
DirectionalLight dlight(glm::vec3(0.f, 10.f, 0.f));

// point light & directional light intensity
float plight_str = .05f;
float dlight_str = .3f;

// For adjusting how fast the submarine object goes 
float submarine_speed = 1.0f;

bool low = true;
bool med = false;
bool high = false;

bool isFirstPerson = false;

/* Contains all model data */
std::vector<Model3D> modelList;

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

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        if (low) {
            low = false;
            med = true;
            plight_str = .5f;
        }
        else if (med) {
            med = false;
            high = true;
            plight_str = 1.f;
        }
        else if (high) {
            high = false;
            low = true;
            plight_str = .05f;
        }
    }

    modelList[0].printDepth();
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
    const char* texture_filenames[textures_count] = { "3D/shark_texture.jpg", "3D/dolphin_texture.jpg",
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

    /* Load the normal map */
    GLuint norm_tex;
    glGenTextures(1, &norm_tex);
    glBindTexture(GL_TEXTURE_2D, norm_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    stbi_set_flip_vertically_on_load(true);

    int img_width2, img_height2, color_channels2;
    unsigned char* norm_bytes = stbi_load("3D/rock_normal.jpg", &img_width2, &img_height2, &color_channels2, 0);
    /* https://www.filterforge.com/filters/1160-normal.jpg */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width2, img_height2, 0, GL_RGB, GL_UNSIGNED_BYTE, norm_bytes);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(norm_bytes);

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

    Shader skyboxShader = Shader("Shaders/skybox.vert", "Shaders/skybox.frag");
    skyboxShader.useShaderProgram();

    /* Shader object for ship with normal maps */
    Shader normalShader = Shader("Shaders/normalmap.vert", "Shaders/normalmap.frag");
   
    /* Vertices for the cube */
    float skyboxVertices[]{
        -1.f, -1.f, 1.f, //0
        1.f, -1.f, 1.f,  //1
        1.f, -1.f, -1.f, //2
        -1.f, -1.f, -1.f,//3
        -1.f, 1.f, 1.f,  //4
        1.f, 1.f, 1.f,   //5
        1.f, 1.f, -1.f,  //6
        -1.f, 1.f, -1.f  //7
    };

    /* Skybox Indices */
    unsigned int skyboxIndices[]{
        1,2,6,
        6,5,1,

        0,4,7,
        7,3,0,

        4,5,6,
        6,7,4,

        0,3,2,
        2,1,0,

        0,1,5,
        5,4,0,

        3,7,6,
        6,2,3
    };

    GLuint skyboxVAO, skyboxVBO, skyboxEBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glGenBuffers(1, &skyboxEBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(skyboxVertices),
        &skyboxVertices,
        GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        sizeof(GL_INT) * 36,
        &skyboxIndices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    
    std::string facesSkybox[]{
        "Skybox/uw_lf.jpg",
        "Skybox/uw_rt.jpg",
        "Skybox/uw_up.jpg",
        "Skybox/uw_dn.jpg",
        "Skybox/uw_ft.jpg",
        "Skybox/uw_bk.jpg", 
    };

    /* Create the cube map */
    unsigned int skyboxTexture;
    glGenTextures(1, &skyboxTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

    /* Prevent pixelating */
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    /* Prevent texture from tiling */
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (unsigned int i = 0; i < 6; i++) {

        int w, h, skyColorChannel;
        if (i == 2) { /* uw_up looks wrong is not flipped */
            stbi_set_flip_vertically_on_load(true);
        } else {
            stbi_set_flip_vertically_on_load(false);
        }
        
        unsigned char* data = stbi_load(facesSkybox[i].c_str(),
            &w, &h, &skyColorChannel, 0);

        if (data) {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE,
                data
            );
            stbi_image_free(data);
        }

    }

    stbi_set_flip_vertically_on_load(true);

    /* Create main object, will be normal mapped. Set last parameter to true as it is normal mapped */
    Model3D mainObj = Model3D("3D/shark.obj", 0.0f, -10.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.03f, 0.03f, 0.03f, 270.0f, true);
    mainObj.rotate_on_axis(-90.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    /* Create object for testing */
    //Model3D testObj = Model3D("3D/cat.obj", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.01f, 0.01f, 0.01f, 270.0f, true);
    //testObj.rotate_on_axis(90.0f, glm::vec3(1.0f, 0.0f, 0.0f));

    modelList.push_back(mainObj);


    /* MODELS AT DEPTH -20.0 */

    /* https://free3d.com/3d-model/-dolphin-v1--12175.html */
    Model3D dolphinObj = Model3D("3D/dolphin.obj", 20.0f, -20.0f, 20.0f, 0.0f, 0.0f, 1.0f, 0.025f, 0.025f, 0.025f, 90.0f, false);
    dolphinObj.rotate_on_axis(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    modelList.push_back(dolphinObj);

    /* https://free3d.com/3d-model/shark-v2--367955.html */
    Model3D sharkObj = Model3D("3D/shark.obj", -20.0f, -20.0f, 20.0f, 0.0f, 0.0f, 1.0f, 0.05f, 0.05f, 0.05f, 90.0f, false);
    sharkObj.rotate_on_axis(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    modelList.push_back(sharkObj);

    /* https://free3d.com/3d-model/-sea-turtle-v1--427786.html */
    Model3D turtleObj = Model3D("3D/turtle.obj", -10.0f, -20.0f, 10.0f, 0.0f, 0.0f, 1.0f, 0.04f, 0.04f, 0.04f, 90.0f, false);
    turtleObj.rotate_on_axis(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    modelList.push_back(turtleObj);

    /* MODELS AT DEPTH -30.0 */

    /* https://free3d.com/3d-model/coral-beauty-angelfish-v1--473554.html */
    Model3D angelfishObj = Model3D("3D/angelfish.obj", 0.0f, -30.0f, 10.0f, 0.0f, 0.0f, 1.0f, 2.0f, 2.0f, 2.0f, 90.0f, false);
    modelList.push_back(angelfishObj);

    /* https://free3d.com/3d-model/coral-v1--901825.html */
    Model3D coralObj = Model3D("3D/coral.obj", -10.0f, -30.0f, 20.0f, 0.0f, 0.0f, 1.0f, 0.1f, 0.1f, 0.1f, 90.0f, false);
    modelList.push_back(coralObj);

    /* https://free3d.com/3d-model/aquarium-deep-sea-diver-v1--436500.html */
    Model3D diverObj = Model3D("3D/diver.obj", 20.0f, -30.0f, -10.0f, 0.0f, 0.0f, 1.0f, 0.3f, 0.3f, 0.3f, 90.0f, false);
    modelList.push_back(diverObj);

    const int modelCount = 7;
    GLuint VAO[modelCount], VBO[modelCount];

    /* Setup VAO and VBOs */
    glGenVertexArrays(modelCount, VAO);
    glGenBuffers(modelCount, VBO);

    /* Initialize buffers for main ship obj with normal maps */
    glBindVertexArray(VAO[0]);
    modelList[0].init_buffers_with_normals(VAO[0], VBO[0]);

    /* Inititalize buffers for rest of obj in list */
    for (int i = 1; i < modelCount; i++) {
        glBindVertexArray(VAO[i]);
        modelList[i].init_buffers(VAO[i], VBO[i]);
    }

    projection_matrix = pcam.GetPer(60.f);
    skybox_projection_matrix = pcam.GetPer(60.f);
    
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
        if (isPers) {
            viewMatrix = camera.GetViewMatrixThird();
            isFirstPerson = false;
        }
        else if (isOrtho) {
            viewMatrix = camera.GetViewMatrixFirst();
            isFirstPerson = false;
        }
        else {
            viewMatrix = camera.GetViewMatrixFirst();
            projection_matrix = pcam.GetPer(30.f);
            skybox_projection_matrix = pcam.GetPer(30.f);
            isFirstPerson = true;
        }

        // light to always follow sub
        plight.lightPos.x = modelList[0].transformation_matrix[3][0];
        plight.lightPos.y = modelList[0].transformation_matrix[3][1];
        plight.lightPos.z = modelList[0].transformation_matrix[3][2] - 2.f;

        /* Render skybox */
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        skyboxShader.useShaderProgram();

        glm::mat4 sky_view = glm::mat4(1.0f);
        sky_view = glm::mat4(glm::mat3(viewMatrix));

        unsigned int sky_projectionLoc = glGetUniformLocation(skyboxShader.getID() , "projection");
        glUniformMatrix4fv(sky_projectionLoc, 1, GL_FALSE, glm::value_ptr(skybox_projection_matrix));

        unsigned int sky_viewLoc = glGetUniformLocation(skyboxShader.getID(), "view");
        glUniformMatrix4fv(sky_viewLoc, 1, GL_FALSE, glm::value_ptr(sky_view));

        unsigned int firstPersonSkyboxLoc = glGetUniformLocation(skyboxShader.getID(), "firstPerson");
        glUniform1f(firstPersonSkyboxLoc, isFirstPerson);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL);
        
        /* Set uniforms in main shader files */
        mainShader.useShaderProgram();

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

        unsigned int firstPersonLoc = glGetUniformLocation(mainShader.getID(), "firstPerson");
        glUniform1f(firstPersonLoc, isFirstPerson);

        /* Set uniforms in normal shader which will be used for rendering the ship */
        normalShader.useShaderProgram();
        /* Point light for normal shader*/
        unsigned int normLightPosLoc = glGetUniformLocation(normalShader.getID(), "lightPos");
        glUniform3fv(normLightPosLoc, 1, glm::value_ptr(plight.lightPos));
        unsigned int normColorLoc = glGetUniformLocation(normalShader.getID(), "ourColor");
        glUniform3fv(normColorLoc, 1, glm::value_ptr(glm::vec3(1, 1, 1)));
        unsigned int normLightColorLoc = glGetUniformLocation(normalShader.getID(), "lightColor");
        glUniform3fv(normLightColorLoc, 1, glm::value_ptr(plight.lightColor));
        unsigned int normConstantLoc = glGetUniformLocation(normalShader.getID(), "constant");
        glUniform1f(normConstantLoc, plight.constant);
        unsigned int normLinearLoc = glGetUniformLocation(normalShader.getID(), "linear");
        glUniform1f(normLinearLoc, plight.linear);
        unsigned int normQuadraticLoc = glGetUniformLocation(normalShader.getID(), "quadratic");
        glUniform1f(normQuadraticLoc, plight.quadratic);
        unsigned int normAmbientStrLoc = glGetUniformLocation(normalShader.getID(), "ambientStr");
        glUniform1f(normAmbientStrLoc, plight.ambientStr);
        unsigned int normAmbientColorLoc = glGetUniformLocation(normalShader.getID(), "ambientColor");
        glUniform3fv(normAmbientColorLoc, 1, glm::value_ptr(plight.ambientColor));
        unsigned int normCameraPosLoc = glGetUniformLocation(normalShader.getID(), "cameraPos");
        glUniform3fv(normCameraPosLoc, 1, glm::value_ptr(camera.Position));
        unsigned int normSpecStrLoc = glGetUniformLocation(normalShader.getID(), "specStr");
        glUniform1f(normSpecStrLoc, plight.specStr);
        unsigned int normSpecPhogLoc = glGetUniformLocation(normalShader.getID(), "specPhong");
        glUniform1f(normSpecPhogLoc, plight.specPhong);
        /* Direction light for normal shader*/
        unsigned int normDirlightPosLoc = glGetUniformLocation(normalShader.getID(), "direction");
        glUniform3fv(normDirlightPosLoc, 1, glm::value_ptr(dlight.direction));
        unsigned int normDirlightColorLoc = glGetUniformLocation(normalShader.getID(), "dirlightColor");
        glUniform3fv(normDirlightColorLoc, 1, glm::value_ptr(dlight.lightColor));
        unsigned int normDirambientStrLoc = glGetUniformLocation(normalShader.getID(), "dirambientStr");
        glUniform1f(normDirambientStrLoc, dlight.ambientStr);
        unsigned int normDirambientColorLoc = glGetUniformLocation(normalShader.getID(), "dirambientColor");
        glUniform3fv(normDirambientColorLoc, 1, glm::value_ptr(dlight.ambientColor));
        unsigned int normDirspecStrLoc = glGetUniformLocation(normalShader.getID(), "dirspecStr");
        glUniform1f(normDirspecStrLoc, dlight.specStr);
        unsigned int normDirspecPhogLoc = glGetUniformLocation(normalShader.getID(), "dirspecPhong");
        glUniform1f(normDirspecPhogLoc, dlight.specPhong);
        /* Rest of uniforms for normal shader */
        unsigned int normFirstPersonLoc = glGetUniformLocation(mainShader.getID(), "firstPerson");
        glUniform1f(normFirstPersonLoc, isFirstPerson);
        unsigned int normProjectionLoc = glGetUniformLocation(normalShader.getID(), "projection");
        glUniformMatrix4fv(normProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
        unsigned int normViewLoc = glGetUniformLocation(normalShader.getID(), "view");
        glUniformMatrix4fv(normViewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
        unsigned int normTransformationLoc = glGetUniformLocation(normalShader.getID(), "transform");
        GLuint normTexOAddress = glGetUniformLocation(normalShader.getID(), "tex0");
        glUniform1i(normTexOAddress, 0);
        GLuint normTex2Address = glGetUniformLocation(normalShader.getID(), "norm_tex");
        glUniform1i(normTex2Address, 1);

        /* Draw submarine object */
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, norm_tex); 

        if (isPers or isOrtho) {
            modelList[0].draw(normTransformationLoc, 0, mainObj.fullVertexData.size() / 14, VAO[0]);
            //modelList[0].printDepth();
        }

        /* Unbind textures */
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);

        /* Use main shader to draw rest of models */
        mainShader.useShaderProgram();

        /* Draw rest of models in dolphin, shark, turtle, angelfish, coral, diver */
        for (int i = 1; i < modelList.size(); i++) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
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
        projection_matrix = pcam.GetPer(60.f);
        skybox_projection_matrix = pcam.GetPer(60.f);
        
    }
    // orthographic view
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        projection_matrix = ocam.GetOrtho();
        skybox_projection_matrix = pcam.GetPer(60.f);
        camera.Pitch = 89.f;
        camera.Yaw = -89.f;
        
    }

    if (isPers) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // forward
        {
            modelList[0].move(glm::vec3(-1.0f * submarine_speed, 0.0f, 0.0f), modelList);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // backward
        {
            modelList[0].move(glm::vec3(submarine_speed, 0.0f, 0.0f), modelList);
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // turn left
        {
            modelList[0].rotate_on_axis(0.007f, glm::vec3(0.0f, 0.0f, 1.0f));
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // turn right
        {
            modelList[0].rotate_on_axis(-0.007f, glm::vec3(0.0f, 0.0f, 1.0f));
        }

        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) // descend
        {   
            modelList[0].move(glm::vec3(0.0f, 0.0f, -1.0f * submarine_speed), modelList);
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) // ascend
        {
            modelList[0].move(glm::vec3(0.0f, 0.0f, submarine_speed), modelList);
        }
    }

    if (isOrtho) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camera.Position.z += .005f;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camera.Position.x += .005f;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camera.Position.z -= .005f;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camera.Position.x -= .005f;
        }
    }
    else if (isOrtho == false) {
        camera.Position.x = modelList[0].transformation_matrix[3][0];
        camera.Position.y = modelList[0].transformation_matrix[3][1];
        camera.Position.z = modelList[0].transformation_matrix[3][2];
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

    /* Apply sensitivity value */
    xoffset *= 0.10f;
    yoffset *= 0.10f;

    lastX = xpos;
    lastY = ypos;

    /* Drag map on click for orthographic top view */
    if (isOrtho) {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            camera.Position.x += xoffset * 0.5f;
            camera.Position.y += yoffset * -2.0f;
        }
        return;
    }

    camera.ProcessMouseMovement(xoffset, yoffset);
}