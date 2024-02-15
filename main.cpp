#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "Skybox.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

glm::vec3 lightPos[13];
GLuint lightPosLoc[13];
int pornesteLuminaDir = 1;
int pornesteLuminaPos = 0;
GLfloat lightAngle = 0.0f;
// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
glm::mat4 lightRotation;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 3.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.5f;

GLboolean pressedKeys[1024];

// models
gps::Model3D scena;
GLfloat angle;

// shaders
gps::Shader myBasicShader;

//animations
gps::Model3D elice1;
gps::Model3D elice2;
gps::Model3D elice3;
gps::Model3D elice4;
float angleElice = 0.0;
float eliceSpeed = 50.0f;
glm::vec3 elicePos1 = glm::vec3(43.7626f, 18.7377f, -75.0011f);
glm::vec3 elicePos2 = glm::vec3(62.1929f, 18.6356f, -102.69f);
glm::vec3 elicePos3 = glm::vec3(74.8171f, 18.6356f, -86.7056f);
glm::vec3 elicePos4 = glm::vec3(93.7696f, 18.6356f, -65.9148f);
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;
bool startEliceAnimation = true;

//skybox
gps::Shader skyBoxShader;
gps::SkyBox myskybox;
gps::SkyBox myskyboxnight;

//shadow
GLuint shadowMapFBO;
GLuint depthMapTexture;
gps::Shader depthMapShader;
gps::Shader lightShader;
const unsigned int SHADOW_WIDTH = 2048*8;
const unsigned int SHADOW_HEIGHT = 2048*8;
bool shadowMap;
gps::Shader screenQuadShader;
gps::Model3D screenQuad;
gps::Model3D lightCube;

GLuint fogDensityLoc;
GLfloat fogDensity;

float circleRadius = 90.0f;
float centerX = 0.0f;
float centerZ = 40.0f;
float angularSpeed = 0.7f;
bool startCameraAnimation = false;

int stopCameraAnimation = 1;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
    if (pressedKeys[GLFW_KEY_L]) {
        if (pornesteLuminaDir == 1) {
            pornesteLuminaDir = 0;
        }
        else {
            pornesteLuminaDir = 1;
        }
        myBasicShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "pornesteLuminaDir"), pornesteLuminaDir);
    }
    if (pressedKeys[GLFW_KEY_K]) {
        if (pornesteLuminaPos == 1) {
            pornesteLuminaPos = 0;
        }
        else {
            pornesteLuminaPos = 1;
        }
        myBasicShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "pornesteLuminaPos"), pornesteLuminaPos);
    }
    if (pressedKeys[GLFW_KEY_Z]) {
        if (stopCameraAnimation == 0) {
            stopCameraAnimation = 1;
        }
        else {
            stopCameraAnimation = 0;
        }
    }
    if (pressedKeys[GLFW_KEY_M]) {
        shadowMap = !shadowMap;
    }
}

float lastX = 400, lastY = 300;

bool firstMouse = 1;
float yaw = -90.0f;
float pitch = 0.0f;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}
float fov = 45.0f;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 80.0f)
        fov = 80.0f;
    myBasicShader.useShaderProgram();
    projection = glm::perspective(glm::radians(fov), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}
void moduriVizualizare()
{
    //solid
    if (pressedKeys[GLFW_KEY_1])
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    //wireframe
    if (pressedKeys[GLFW_KEY_2])
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    //poligonal
    if (pressedKeys[GLFW_KEY_3])
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    //smooth
    if (pressedKeys[GLFW_KEY_4]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_POLYGON_SMOOTH);
    }
}

void introCameraAnimation(gps::Camera& camera, float deltaTime) {
    static float elapsedTime = 0.0f;

    if (stopCameraAnimation == true && elapsedTime < 20.0f) {
        float viteza = 0.3f;
        float raza = 100.0f;
        glm::vec3 centru(0.0f, 50.0f, 0.0f);

        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), elapsedTime * viteza, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec4 relativePosition = rotationMatrix * glm::vec4(0.0f, 0.0f, -raza, 1.0f);
        glm::vec3 targetPosition = centru + glm::vec3(relativePosition);

        float lerpFactor = 0.1f;
        glm::vec3 cameraPosition = glm::mix(camera.getPosition(), targetPosition, lerpFactor);

        camera.setPosition(cameraPosition);

        elapsedTime += deltaTime;
    }
    else {
        if (stopCameraAnimation == true) {
            stopCameraAnimation = false;
            elapsedTime = 0.0f; 
            camera.setPosition(glm::vec3(0.0f, 3.0f, 3.0f));
        }

        glm::mat4 view = camera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }
    
}
void processMovement() {

   
    moduriVizualizare();
    myBasicShader.useShaderProgram();
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "pornesteLuminaDir"), pornesteLuminaDir);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "pornesteLuminaPos"), pornesteLuminaPos);
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        glm::vec3 pozitie = myCamera.getPosition();
        if (pozitie.y < 1.0f) {
            myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        }
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        glm::vec3 pozitie = myCamera.getPosition();
        if (pozitie.y < 1.0f) {
            myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        }
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }
    
    if (pressedKeys[GLFW_KEY_F]) {
        fogDensity += 0.002f;
        //std::cout << fogDensity<<"\n";
        if (fogDensity > 0.3f) {
            fogDensity = 0.3f;
        }
        myBasicShader.useShaderProgram();
        glUniform1fv(fogDensityLoc, 1, &fogDensity);
    }
    if (pressedKeys[GLFW_KEY_G]) {
        fogDensity -= 0.002f;
        if (fogDensity < 0.0f) {
            fogDensity = 0.0f;
        }
        myBasicShader.useShaderProgram();
        glUniform1fv(fogDensityLoc, 1, &fogDensity);
    }
    if (pressedKeys[GLFW_KEY_H]) {
        lightAngle += 0.5f;
    }
    if (pressedKeys[GLFW_KEY_J]) {
        lightAngle -= 0.5f;
    }
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetScrollCallback(myWindow.getWindow(), scroll_callback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    scena.LoadModel("models/scena3D/scena3D17.obj");
    elice1.LoadModel("models/scena3D/animations/elice1.obj");
    elice2.LoadModel("models/scena3D/animations/elice2.obj");
    elice3.LoadModel("models/scena3D/animations/elice3.obj");
    elice4.LoadModel("models/scena3D/animations/elice4.obj");
    screenQuad.LoadModel("models/quad/quad.obj");
    lightCube.LoadModel("models/cube/cube.obj");
}

void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    myBasicShader.useShaderProgram();
    skyBoxShader.loadShader("shaders/skyBoxShader.vert", "shaders/skyBoxShader.frag");
    skyBoxShader.useShaderProgram();
    depthMapShader.loadShader(
        "shaders/depthMapShader.vert",
        "shaders/depthMapShader.frag");
    depthMapShader.useShaderProgram();
    screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    screenQuadShader.useShaderProgram();
    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    lightShader.useShaderProgram();
}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(fov),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 500.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(-120.0f, 120.0f, 120.0f);
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));


	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    lightPos[0] = glm::vec3(-4.38577f, 4.51597f, 34.0541f);
    lightPosLoc[0] = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos[0]");
    glUniform3fv(lightPosLoc[0], 1, glm::value_ptr(lightPos[0]));
    lightPos[1] = glm::vec3(7.69992f, 4.51597f, 24.5732f);
    lightPosLoc[1] = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos[1]");
    glUniform3fv(lightPosLoc[1], 1, glm::value_ptr(lightPos[1]));
    lightPos[2] = glm::vec3(18.6732f, 4.45459f, 19.3372f);
    lightPosLoc[2] = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos[2]");
    glUniform3fv(lightPosLoc[2], 1, glm::value_ptr(lightPos[2]));
    lightPos[3] = glm::vec3(28.2782f, 4.51831f, 16.0345f);
    lightPosLoc[3] = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos[3]");
    glUniform3fv(lightPosLoc[3], 1, glm::value_ptr(lightPos[3]));
    lightPos[4] = glm::vec3(37.0316f, 4.52271f, 13.9133f);
    lightPosLoc[4] = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos[4]");
    glUniform3fv(lightPosLoc[4], 1, glm::value_ptr(lightPos[4]));
    lightPos[5] = glm::vec3(46.754f, 4.43476f, 12.5834f);
    lightPosLoc[5] = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos[5]");
    glUniform3fv(lightPosLoc[5], 1, glm::value_ptr(lightPos[5]));
    lightPos[6] = glm::vec3(60.2365f, 4.86586f, 11.7257f);
    lightPosLoc[6] = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos[6]");
    glUniform3fv(lightPosLoc[6], 1, glm::value_ptr(lightPos[6]));
    lightPos[7] = glm::vec3(73.4612f, 4.45797f, 11.2487f);
    lightPosLoc[7] = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos[7]");
    glUniform3fv(lightPosLoc[7], 1, glm::value_ptr(lightPos[7]));
    lightPos[8] = glm::vec3(92.0053f, 4.48855f, 12.4302f);
    lightPosLoc[8] = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos[8]");
    glUniform3fv(lightPosLoc[8], 1, glm::value_ptr(lightPos[8]));
    lightPos[9] = glm::vec3(111.271f, 4.46423f, 14.7157f);
    lightPosLoc[9] = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos[9]");
    glUniform3fv(lightPosLoc[9], 1, glm::value_ptr(lightPos[9]));
    lightPos[10] = glm::vec3(43.233f, 4.76243f, 60.1071f);
    lightPosLoc[10] = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos[10]");
    glUniform3fv(lightPosLoc[10], 1, glm::value_ptr(lightPos[10]));
    lightPos[11] = glm::vec3(17.9279f, 4.76243f, 60.1439f);
    lightPosLoc[11] = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos[11]");
    glUniform3fv(lightPosLoc[11], 1, glm::value_ptr(lightPos[11]));
    lightPos[12] = glm::vec3(17.9279f, 4.76243f, 60.1439f);
    lightPosLoc[12] = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos[12]");
    glUniform3fv(lightPosLoc[12], 1, glm::value_ptr(lightPos[12]));


    fogDensity = 0.0f;
    fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
    glUniform1fv(fogDensityLoc, 1, &fogDensity);

}
void initFBO() {
    //TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
    glGenFramebuffers(1, &shadowMapFBO);
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
        0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
void initSkybox() {
    std::vector<const GLchar*> faces;
    faces.push_back("Skybox/day/right.jpg");
    faces.push_back("Skybox/day/left.jpg");
    faces.push_back("Skybox/day/top.jpg");
    faces.push_back("Skybox/day/bottom.jpg");
    faces.push_back("Skybox/day/back.jpg");
    faces.push_back("Skybox/day/front.jpg");
    myskybox.Load(faces);
}
void initSkyboxNight() {
    std::vector<const GLchar*> faces;
    faces.push_back("Skybox/night/nightsky_rt.tga");
    faces.push_back("Skybox/night/nightsky_lf.tga");
    faces.push_back("Skybox/night/nightsky_up.tga");
    faces.push_back("Skybox/night/nightsky_dn.tga");
    faces.push_back("Skybox/night/nightsky_bk.tga");
    faces.push_back("Skybox/night/nightsky_ft.tga");
    myskyboxnight.Load(faces);
}
void drawEliceMoara1(gps::Shader shader, bool shadow)
{

    glm::mat4 eliceModelTr = glm::translate(glm::mat4(1.0f), elicePos1);
    glm::mat4 eliceModelRo = glm::rotate(glm::mat4(1.0f), glm::radians(angleElice), glm::vec3(0.0f, 0.0f, -1.0f));
    glm::mat4 eliceModelTri = glm::translate(glm::mat4(1.0f), -elicePos1);
    glm::mat4 eliceModel = model * eliceModelTr * eliceModelRo * eliceModelTri;

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(eliceModel));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    if (shadow == false) {
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    elice1.Draw(shader);
}
void drawEliceMoara2(gps::Shader shader, bool shadow)
{

    glm::mat4 eliceModelTr = glm::translate(glm::mat4(1.0f), elicePos2);
    glm::mat4 eliceModelRo = glm::rotate(glm::mat4(1.0f), glm::radians(angleElice), glm::vec3(0.0f, 0.0f, -1.0f));
    glm::mat4 eliceModelTri = glm::translate(glm::mat4(1.0f), -elicePos2);
    glm::mat4 eliceModel = model * eliceModelTr * eliceModelRo * eliceModelTri;

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(eliceModel));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    if (shadow == false) {
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    elice2.Draw(shader);
}
void drawEliceMoara3(gps::Shader shader, bool shadow)
{

    glm::mat4 eliceModelTr = glm::translate(glm::mat4(1.0f), elicePos3);
    glm::mat4 eliceModelRo = glm::rotate(glm::mat4(1.0f), glm::radians(angleElice), glm::vec3(0.0f, 0.0f, -1.0f));
    glm::mat4 eliceModelTri = glm::translate(glm::mat4(1.0f), -elicePos3);
    glm::mat4 eliceModel = model * eliceModelTr * eliceModelRo * eliceModelTri;

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(eliceModel));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    if (shadow == false) {
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    elice3.Draw(shader);
}
void drawEliceMoara4(gps::Shader shader, bool shadow)
{

    glm::mat4 eliceModelTr = glm::translate(glm::mat4(1.0f), elicePos4);
    glm::mat4 eliceModelRo = glm::rotate(glm::mat4(1.0f), glm::radians(angleElice), glm::vec3(0.0f, 0.0f, -1.0f));
    glm::mat4 eliceModelTri = glm::translate(glm::mat4(1.0f), -elicePos4);
    glm::mat4 eliceModel = model * eliceModelTr * eliceModelRo * eliceModelTri;

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(eliceModel));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    if (shadow == false) {
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    elice4.Draw(shader);
}
void eliceMoaraAnimation()
{
    angleElice += eliceSpeed * deltaTime;
}
void renderScena(gps::Shader shader, bool shadow) {
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!shadow) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    scena.Draw(shader);
}
glm::mat4 computeLightSpaceTrMatrix() {
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 lightView = glm::lookAt(glm::vec3(lightRotation * glm::vec4(lightDir, 1.0f)), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = -200.0f, far_plane = 400.0f;
    glm::mat4 lightProjection = glm::ortho(-400.0f, 400.0f, -200.0f, 200.0f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

    return lightSpaceTrMatrix;
}
float animationTime = 0;
void renderScene() {
	
    if (startEliceAnimation)
    {
        eliceMoaraAnimation();
    }
    introCameraAnimation(myCamera, deltaTime);
    //if (stopCameraAnimation) {
    //    const float animationDuration = 25.0f; // Durata totală a animației în secunde
    //    animationTime += deltaTime;

    //    if (animationTime > animationDuration) {
    //        stopCameraAnimation = false;
    //        animationTime = 0.0f;
    //    }

    //    float t = animationTime / animationDuration;
    //    float angle = glm::radians(360.0f * t); // Unghiul de rotație în funcție de timp

    //    // Calculează poziția camerei pe un cerc în jurul punctului (0.0f, 3.0f, 3.0f)
    //    float radius = 8.0f;
    //    float camX = radius * cos(angle);
    //    float camZ = radius * sin(angle);

    //    myCamera = gps::Camera(glm::vec3(camX, 200.0f, camZ ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //}
   
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    renderScena(depthMapShader,true);
    drawEliceMoara1(depthMapShader, true);
    drawEliceMoara2(depthMapShader, true);
    drawEliceMoara3(depthMapShader, true);
    drawEliceMoara4(depthMapShader, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    if (shadowMap) {
        glViewport(0, 0, (float)myWindow.getWindowDimensions().width, (float)myWindow.getWindowDimensions().height);

        glClear(GL_COLOR_BUFFER_BIT);

        screenQuadShader.useShaderProgram();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

        glDisable(GL_DEPTH_TEST);
        screenQuad.Draw(screenQuadShader);
        glEnable(GL_DEPTH_TEST);
    }
    else {

        glViewport(0, 0, (float)myWindow.getWindowDimensions().width, (float)myWindow.getWindowDimensions().height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myBasicShader.useShaderProgram();

        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

        //bind the shadow map
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

        glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(computeLightSpaceTrMatrix()));
        myBasicShader.useShaderProgram();

        renderScena(myBasicShader, false);
        drawEliceMoara1(myBasicShader, false);
        drawEliceMoara2(myBasicShader, false);
        drawEliceMoara3(myBasicShader, false);
        drawEliceMoara4(myBasicShader, false);
        if (pornesteLuminaDir == 1) {
            myskybox.Draw(skyBoxShader, view, projection);
        }
        else {
            myskyboxnight.Draw(skyBoxShader, view, projection);
        }
    }
}

void cleanup() {
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
    myWindow.Delete();
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    setWindowCallbacks();
    initSkybox();
    initSkyboxNight();
    initFBO();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
