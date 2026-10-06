#define GLEW_DLL
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Model.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 cameraPos = glm::vec3(0.0f, 4.0f, 12.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, -0.2f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f, pitch = 0.0f;
float lastX = 400, lastY = 300;
bool firstMouse = true;
float deltaTime = 0.0f, lastFrame = 0.0f;

float yawPlatform = 0.0f;         
float vertOffset = 0.0f;
float extOffset = 0.0f;

glm::mat4 matPlatform(1.0f);     
glm::mat4 matVert(1.0f);
glm::mat4 matManip(1.0f);

const glm::vec3 pivotOffset = glm::vec3(0.0f, 0.0f, 0.0f);

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float camSpeed = 5.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += camSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= camSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * camSpeed;
   
    float rotSpeed = 50.0f * deltaTime;
    float moveSpeed = 2.0f * deltaTime;

    // 1.
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) yawPlatform += rotSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) yawPlatform -= rotSpeed;
    yawPlatform = glm::clamp(yawPlatform, -180.0f, 180.0f);

    // 2.
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) vertOffset += moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) vertOffset -= moveSpeed;
    vertOffset = glm::clamp(vertOffset, -0.135f, 0.6f);

    // 3.
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) extOffset += moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) extOffset -= moveSpeed;
    extOffset = glm::clamp(extOffset, -0.8f, 0.8f);

    matPlatform = glm::mat4(1.0f);
    matPlatform = glm::translate(matPlatform, pivotOffset);
    matPlatform = glm::rotate(matPlatform, glm::radians(yawPlatform), glm::vec3(0, 1, 0));
    matPlatform = glm::translate(matPlatform, -pivotOffset);

    matVert = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, vertOffset, 0.0f));

    matManip = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, extOffset));
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) { lastX = (float)xpos; lastY = (float)ypos; firstMouse = false; }
    float xoff = (float)xpos - lastX;
    float yoff = lastY - (float)ypos;
    lastX = (float)xpos; lastY = (float)ypos;

    yaw += xoff * 0.1f;
    pitch += yoff * 0.1f;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    glm::vec3 f;
    f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    f.y = sin(glm::radians(pitch));
    f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(f);
}

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lab 7 - Mechanical Transforms", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewInit();
    glEnable(GL_DEPTH_TEST);

    Shader ourShader("vertex_shader.glsl", "fragment_shader.glsl");

    Model base("base.obj");    
    Model horizon("arm.obj"); 
    Model vertical("platform.obj");     
    Model grip("grip.obj");   

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();
        ourShader.setVec3("lightPos", glm::vec3(1.2f, 1.0f, 2.0f));
        ourShader.setVec3("viewPos", cameraPos);
        ourShader.setVec3("lightColor", glm::vec3(1.0f));
        ourShader.setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));

        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        ourShader.setMat4("model", model);
        base.Draw(ourShader);

        model = matPlatform;
        ourShader.setMat4("model", model);
        vertical.Draw(ourShader);  

        model = matPlatform * matVert;
        ourShader.setMat4("model", model);
        horizon.Draw(ourShader);  

        model = matPlatform * matVert * matManip;
        ourShader.setMat4("model", model);
        grip.Draw(ourShader);

        //—брос матрицы 
        model = glm::mat4(1.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}