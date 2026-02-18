#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <gdev.h>

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE  "Lighting Cube"

GLFWwindow *pWindow;

GLuint vao;
GLuint vbo;
GLuint shader;
GLuint textureID;

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

float lastX = 320, lastY = 180;
float yaw = -90.0f, pitch = 0.0f;
float fov = 45.0f;
bool firstMouse = true;

// Simple light
glm::vec3 lightPosition(1.0f, 2.0f, 1.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
float specularity = 0.5f;
float lightHeight = 5.0f;

// Spotlight
glm::vec3 spotPosition(1.0f, 5.0f, 1.0f);
glm::vec3 spotDirection(-1.0f, -1.0f, -1.0f);
float spotX = 1.0f;
float spotZ = 1.0f;

// Simple cube (6 faces, 2 triangles each)
float cube[] =
{
    // positions                // color                // tex          // normal
    // Front face (positive Z)
    -30.0f, -15.0f, 18.75f,     1.00f, 1.00f, 1.00f,    0.0f, 0.0f,     0.0f, 0.0f, 1.0f,
    30.0f, -15.0f, 18.75f,      1.00f, 1.00f, 1.00f,    1.0f, 0.0f,     0.0f, 0.0f, 1.0f,
    30.0f, 15.0f, 18.75f,       1.00f, 1.00f, 1.00f,    1.0f, 1.0f,     0.0f, 0.0f, 1.0f,
    -30.0f, -15.0f, 18.75f,     1.00f, 1.00f, 1.00f,    0.0f, 0.0f,     0.0f, 0.0f, 1.0f,
    30.0f, 15.0f, 18.75f,       1.00f, 1.00f, 1.00f,    1.0f, 1.0f,     0.0f, 0.0f, 1.0f,
    -30.0f, 15.0f, 18.75f,      1.00f, 1.00f, 1.00f,    0.0f, 1.0f,     0.0f, 0.0f, 1.0f,


    // Back face (negative Z)
    30.0f, -15.0f, -18.75f,     1.00f, 1.00f, 1.00f,    0.0f, 0.0f,     0.0f, 0.0f, -1.0f,
    -30.0f, -15.0f, -18.75f,    1.00f, 1.00f, 1.00f,    1.0f, 0.0f,     0.0f, 0.0f, -1.0f,
    -30.0f, 15.0f, -18.75f,     1.00f, 1.00f, 1.00f,    1.0f, 1.0f,     0.0f, 0.0f, -1.0f,
    30.0f, -15.0f, -18.75f,     1.00f, 1.00f, 1.00f,    0.0f, 0.0f,     0.0f, 0.0f, -1.0f,
    -30.0f, 15.0f, -18.75f,     1.00f, 1.00f, 1.00f,    1.0f, 1.0f,     0.0f, 0.0f, -1.0f,
    30.0f, 15.0f, -18.75f,      1.00f, 1.00f, 1.00f,    0.0f, 1.0f,     0.0f, 0.0f, -1.0f,


    // Left face (negative X)
    -30.0f, -15.0f, -18.75f,    1.00f, 1.00f, 1.00f,    0.0f, 0.0f,     -1.0f, 0.0f, 0.0f,
    -30.0f, -15.0f, 18.75f,     1.00f, 1.00f, 1.00f,    1.0f, 0.0f,     -1.0f, 0.0f, 0.0f,
    -30.0f, 15.0f, 18.75f,      1.00f, 1.00f, 1.00f,    1.0f, 1.0f,     -1.0f, 0.0f, 0.0f,
    -30.0f, -15.0f, -18.75f,    1.00f, 1.00f, 1.00f,    0.0f, 0.0f,     -1.0f, 0.0f, 0.0f,
    -30.0f, 15.0f, 18.75f,      1.00f, 1.00f, 1.00f,    1.0f, 1.0f,     -1.0f, 0.0f, 0.0f,
    -30.0f, 15.0f, -18.75f,     1.00f, 1.00f, 1.00f,    0.0f, 1.0f,     -1.0f, 0.0f, 0.0f,


    // Right face (positive X)
    30.0f, -15.0f, 18.75f,      1.00f, 1.00f, 1.00f,    0.0f, 0.0f,     1.0f, 0.0f, 0.0f,
    30.0f, -15.0f, -18.75f,     1.00f, 1.00f, 1.00f,    1.0f, 0.0f,     1.0f, 0.0f, 0.0f,
    30.0f, 15.0f, -18.75f,      1.00f, 1.00f, 1.00f,    1.0f, 1.0f,     1.0f, 0.0f, 0.0f,
    30.0f, -15.0f, 18.75f,      1.00f, 1.00f, 1.00f,    0.0f, 0.0f,     1.0f, 0.0f, 0.0f,
    30.0f, 15.0f, -18.75f,      1.00f, 1.00f, 1.00f,    1.0f, 1.0f,     1.0f, 0.0f, 0.0f,
    30.0f, 15.0f, 18.75f,       1.00f, 1.00f, 1.00f,    0.0f, 1.0f,     1.0f, 0.0f, 0.0f,


    // Top face (positive Y)
    -30.0f, 15.0f, 18.75f,      1.00f, 1.00f, 1.00f,    0.0f, 0.0f,     0.0f, 1.0f, 0.0f,
    30.0f, 15.0f, 18.75f,       1.00f, 1.00f, 1.00f,    1.0f, 0.0f,     0.0f, 1.0f, 0.0f,
    30.0f, 15.0f, -18.75f,      1.00f, 1.00f, 1.00f,    1.0f, 1.0f,     0.0f, 1.0f, 0.0f,
    -30.0f, 15.0f, 18.75f,      1.00f, 1.00f, 1.00f,    0.0f, 0.0f,     0.0f, 1.0f, 0.0f,
    30.0f, 15.0f, -18.75f,      1.00f, 1.00f, 1.00f,    1.0f, 1.0f,     0.0f, 1.0f, 0.0f,
    -30.0f, 15.0f, -18.75f,     1.00f, 1.00f, 1.00f,    0.0f, 1.0f,     0.0f, 1.0f, 0.0f,

    // Bottom face (negative Y)
    -30.0f, -15.0f, -18.75f,    1.00f, 1.00f, 1.00f,    0.0f, 0.0f,     0.0f, -1.0f, 0.0f,
    30.0f, -15.0f, -18.75f,     1.00f, 1.00f, 1.00f,    1.0f, 0.0f,     0.0f, -1.0f, 0.0f,
    30.0f, -15.0f, 18.75f,      1.00f, 1.00f, 1.00f,    1.0f, 1.0f,     0.0f, -1.0f, 0.0f,
    -30.0f, -15.0f, -18.75f,    1.00f, 1.00f, 1.00f,    0.0f, 0.0f,     0.0f, -1.0f, 0.0f,
    30.0f, -15.0f, 18.75f,      1.00f, 1.00f, 1.00f,    1.0f, 1.0f,     0.0f, -1.0f, 0.0f,
    -30.0f, -15.0f, 18.75f,     1.00f, 1.00f, 1.00f,    0.0f, 1.0f,     0.0f, -1.0f, 0.0f,
};

bool setup()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    // texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    // normal
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)(8*sizeof(float)));
    glEnableVertexAttribArray(3);

    shader = gdevLoadShader("ex2.vs", "ex2.fs");
    if (!shader) return false;

    textureID = gdevLoadTexture("brickwalltexture.jpg", GL_REPEAT, true, true);
    if (!textureID) return false;

    glEnable(GL_DEPTH_TEST);
    return true;
}

void render()
{
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader);

    lightPosition.y = lightHeight;
    spotPosition = glm::vec3(spotX, spotPosition.y, spotZ);

    // Light uniforms
    glUniform3fv(glGetUniformLocation(shader, "lightPosition"), 1, &lightPosition[0]);
    glUniform3fv(glGetUniformLocation(shader, "lightColor"), 1, &lightColor[0]);
    glUniform1f(glGetUniformLocation(shader, "specColor"), specularity);

    glUniform3fv(glGetUniformLocation(shader, "spotPosition"), 1, &spotPosition[0]);
    glUniform3fv(glGetUniformLocation(shader, "spotDirection"), 1, &spotDirection[0]);
    glUniform1f(glGetUniformLocation(shader, "spotCutoff"), glm::cos(glm::radians(15.0f)));
    glUniform3f(glGetUniformLocation(shader, "spotColor"), 1.0f, 1.0f, 1.0f);

    glUniform3fv(glGetUniformLocation(shader, "cameraPos"), 1, &cameraPos[0]);


    // Projection
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)WINDOW_WIDTH / WINDOW_HEIGHT,
        0.1f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projectionTransform"),
                       1, GL_FALSE, glm::value_ptr(projection));

    // View
    glm::mat4 view = glm::lookAt(
        cameraPos,
        cameraPos + cameraFront,
        cameraUp);
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewTransform"),
                       1, GL_FALSE, glm::value_ptr(view));

    // Model
    glm::mat4 model = glm::mat4(1.0f);
    float scale = 0.1f;
    model = glm::scale(model, glm::vec3(scale, scale, scale));

    glUniformMatrix4fv(glGetUniformLocation(shader, "modelTransform"),
                       1, GL_FALSE, glm::value_ptr(model));

    glm::mat4 normal = glm::transpose(glm::inverse(model));
    glUniformMatrix4fv(glGetUniformLocation(shader, "normalTransform"),
                       1, GL_FALSE, glm::value_ptr(normal));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glBindVertexArray(vao);

    glDrawArrays(GL_TRIANGLES, 0, sizeof(cube)/11*sizeof(float)); // 6 faces × 6 vertices
}

void handleKeys(GLFWwindow* pWindow, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(pWindow, GL_TRUE);
}

void handleResize(GLFWwindow* pWindow, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    pWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (!pWindow) return -1;

    glfwMakeContextCurrent(pWindow);
    glfwSetKeyCallback(pWindow, handleKeys);
    glfwSetFramebufferSizeCallback(pWindow, handleResize);
    glfwSetCursorPosCallback(pWindow, mouse_callback);
    glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    if (setup())
    {
        while (!glfwWindowShouldClose(pWindow))
        {
            processInput(pWindow);
            render();
            glfwSwapBuffers(pWindow);
            glfwPollEvents();
        }
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;  
    const float cameraSpeed = 2.5f * deltaTime; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
        lightColor -= 0.05f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
        lightColor += 0.05f;
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
        lightHeight += 0.05f;
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
        lightHeight -= 0.05f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        spotX += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        spotX -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        spotZ += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        spotZ -= 0.01f;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
};