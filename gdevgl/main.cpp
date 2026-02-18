/******************************************************************************
 * This demo draws a textured quadrilateral on screen, plus the user can change
 * its position, rotation, and scaling using the WASD and arrow keys.
 *
 * The main differences from the previous demo are:
 * - Transformation matrices are now passed to the vertex shader (as uniforms).
 * - The OpenGL GL_BLEND feature is enabled to allow for drawing transparent
 *   texels using the alpha channel.
 *
 * Happy hacking! - eric
 *****************************************************************************/

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <gdev.h>

// change this to your desired window attributes
#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE  "Hello Transform (use WASD and arrow keys)"
GLFWwindow *pWindow;

// --- Lighting variables (from box.cpp, adapted) ---
glm::vec3 lightPosition(1.0f, 5.0f, 1.0f);
glm::vec3 lightColor(100.0f, 100.0f, 100.0f); // Much brighter starting light
float specularity = 0.7f;
float lightHeight = 5.0f;

// Spotlight
glm::vec3 spotPosition(1.0f, 5.0f, 1.0f);
glm::vec3 spotDirection(0.0f, -1.0f, 0.0f);
float spotX = 1.0f;
float spotZ = 1.0f;

// Cube faces - 6 sprites forming a 60x30x37.5 unit cube (15x as big as chest ~4x2x2.5)
// Positioned to contain the existing objects inside
// Front face (positive Z)
float cube_front[] =
{
    // position        color           texcoord    normal
    -30.0f, -15.0f, 18.75f,  1.00f, 1.00f, 1.00f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
    30.0f, -15.0f, 18.75f,   1.00f, 1.00f, 1.00f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
    30.0f, 15.0f, 18.75f,    1.00f, 1.00f, 1.00f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
    -30.0f, -15.0f, 18.75f,  1.00f, 1.00f, 1.00f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
    30.0f, 15.0f, 18.75f,    1.00f, 1.00f, 1.00f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
    -30.0f, 15.0f, 18.75f,   1.00f, 1.00f, 1.00f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
};

// Back face (negative Z)
float cube_back[] =
{
    30.0f, -15.0f, -18.75f,   1.00f, 1.00f, 1.00f,  0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
    -30.0f, -15.0f, -18.75f,  1.00f, 1.00f, 1.00f,  1.0f, 0.0f,   0.0f, 0.0f, -1.0f,
    -30.0f, 15.0f, -18.75f,   1.00f, 1.00f, 1.00f,  1.0f, 1.0f,   0.0f, 0.0f, -1.0f,
    30.0f, -15.0f, -18.75f,   1.00f, 1.00f, 1.00f,  0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
    -30.0f, 15.0f, -18.75f,   1.00f, 1.00f, 1.00f,  1.0f, 1.0f,   0.0f, 0.0f, -1.0f,
    30.0f, 15.0f, -18.75f,    1.00f, 1.00f, 1.00f,  0.0f, 1.0f,   0.0f, 0.0f, -1.0f,
};

// Left face (negative X)
float cube_left[] =
{
    -30.0f, -15.0f, -18.75f,  1.00f, 1.00f, 1.00f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
    -30.0f, -15.0f, 18.75f,   1.00f, 1.00f, 1.00f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
    -30.0f, 15.0f, 18.75f,    1.00f, 1.00f, 1.00f,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
    -30.0f, -15.0f, -18.75f,  1.00f, 1.00f, 1.00f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
    -30.0f, 15.0f, 18.75f,    1.00f, 1.00f, 1.00f,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
    -30.0f, 15.0f, -18.75f,   1.00f, 1.00f, 1.00f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
};

// Right face (positive X)
float cube_right[] =
{
    30.0f, -15.0f, 18.75f,   1.00f, 1.00f, 1.00f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
    30.0f, -15.0f, -18.75f,  1.00f, 1.00f, 1.00f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
    30.0f, 15.0f, -18.75f,   1.00f, 1.00f, 1.00f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
    30.0f, -15.0f, 18.75f,   1.00f, 1.00f, 1.00f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
    30.0f, 15.0f, -18.75f,   1.00f, 1.00f, 1.00f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
    30.0f, 15.0f, 18.75f,    1.00f, 1.00f, 1.00f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
};

// Top face (positive Y)
float cube_top[] =
{
    -30.0f, 15.0f, 18.75f,   1.00f, 1.00f, 1.00f,  0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
    30.0f, 15.0f, 18.75f,    1.00f, 1.00f, 1.00f,  1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
    30.0f, 15.0f, -18.75f,   1.00f, 1.00f, 1.00f,  1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
    -30.0f, 15.0f, 18.75f,   1.00f, 1.00f, 1.00f,  0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
    30.0f, 15.0f, -18.75f,   1.00f, 1.00f, 1.00f,  1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
    -30.0f, 15.0f, -18.75f,  1.00f, 1.00f, 1.00f,  0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
};

// Bottom face (negative Y)
float cube_bottom[] =
{
    -30.0f, -15.0f, -18.75f,  1.00f, 1.00f, 1.00f,  0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
    30.0f, -15.0f, -18.75f,   1.00f, 1.00f, 1.00f,  1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
    30.0f, -15.0f, 18.75f,    1.00f, 1.00f, 1.00f,  1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
    -30.0f, -15.0f, -18.75f,  1.00f, 1.00f, 1.00f,  0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
    30.0f, -15.0f, 18.75f,    1.00f, 1.00f, 1.00f,  1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
    -30.0f, -15.0f, 18.75f,   1.00f, 1.00f, 1.00f,  0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
};


const int NUM_MODELS = 8;
std::vector<float> vertices[NUM_MODELS];

// define OpenGL object IDs to represent the vertex array, shader program, and texture in the GPU
GLuint vao[NUM_MODELS];         // vertex array object (stores the render state for our vertex array)
GLuint vbo[NUM_MODELS];         // vertex buffer object (reserves GPU memory for our vertex array)
GLuint shader;      // combined vertex and fragment shader
GLuint texture[NUM_MODELS];     // texture object

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// variables controlling the camera position and rotation
float lastX = 320, lastY = 180;
float yaw = -90.0f, pitch = 0.0f;
float fov = 45.0f;
bool firstMouse = true;
double previousTime = 0.0;

// Animation points for coin
glm::vec3 pathPointA = glm::vec3(-20.0f, -11.5f, -15.0f);
glm::vec3 pathPointB = glm::vec3(20.0f, -11.5f, -15.0f);
glm::vec3 pathPointC = glm::vec3(0.0f, -11.5f, 15.0f);

// Animation points for chest
glm::vec3 chestPathPointA = glm::vec3(-20.0f, -13.5f, -15.0f);
glm::vec3 chestPathPointB = glm::vec3(20.0f, -13.5f, -15.0f);
glm::vec3 chestPathPointC = glm::vec3(0.0f, -13.5f, 15.0f);

float coinPathTime = 0.0f;
int coinPathSegment = 1;
float coinPathDuration = 4.0f;
glm::vec3 coinPos = pathPointA;

float chestPathTime = 0.0f;
int chestPathSegment = 0;
float chestPathDuration = 4.0f;
glm::vec3 chestPos = chestPathPointA;

void load_model(const char* filename, std::vector<float>& vertices)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }

    float pos[3], color[3], tex[2], norm[3];

    while(
        file >> pos[0] >> pos[1] >> pos[2] >> 
        norm[0] >> norm[1] >> norm[2] >>
        color[0] >> color[1] >> color[2] >> 
        tex[0] >> tex[1]
    ) {
        vertices.push_back(pos[0]);
        vertices.push_back(pos[1]);
        vertices.push_back(pos[2]);
        vertices.push_back(color[0]);
        vertices.push_back(color[1]);
        vertices.push_back(color[2]);
        vertices.push_back(tex[0]);
        vertices.push_back(tex[1]);
        vertices.push_back(norm[0]);
        vertices.push_back(norm[1]);
        vertices.push_back(norm[2]);
    }
}

// Helper function to get position at a given time along the path
glm::vec3 getCoinPositionOnPath(float pathTime, int pathSegment)
{
    float t = pathTime;
    
    glm::vec3 start, end;
    
    if (pathSegment == 0) {
        start = pathPointA;
        end = pathPointB;
    } else if (pathSegment == 1) {
        start = pathPointB;
        end = pathPointC;
    } else {
        start = pathPointC;
        end = pathPointA;
    }
    return glm::mix(start, end, t);
}

glm::vec3 getChestPositionOnPath(float pathTime, int pathSegment)
{
    float t = pathTime;
    
    glm::vec3 start, end;
    
    if (pathSegment == 0) {
        start = pathPointA;
        end = pathPointB;
    } else if (pathSegment == 1) {
        start = pathPointB;
        end = pathPointC;
    } else {
        start = pathPointC;
        end = pathPointA;
    }
    return glm::mix(start, end, t);
}


glm::vec3 getNextWaypoint(int pathSegment)
{
    if (pathSegment == 0) {
        return pathPointB;
    } else if (pathSegment == 1) {
        return pathPointC;
    } else {
        return pathPointA;
    }
}

glm::mat4 getLookAtRotation(glm::vec3 from, glm::vec3 to)
{
    glm::vec3 forward = glm::normalize(to - from);
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up = glm::cross(right, forward);
    
    glm::mat4 rotation(1.0f);
    rotation[0] = glm::vec4(right, 0.0f);
    rotation[1] = glm::vec4(up, 0.0f);
    rotation[2] = glm::vec4(-forward, 0.0f);
    
    return rotation;
}

// called by the main function to do initial setup, such as uploading vertex
// arrays, shader programs, etc.; returns true if successful, false otherwise
bool setup()
{
    // function to load object data from a file (implementation omitted for brevity)
    load_model("coinarray.txt", vertices[0]);
    load_model("chestarray.txt", vertices[1]);
    
    // Load cube face vertices from static arrays
    vertices[2].assign(cube_front, cube_front + sizeof(cube_front) / sizeof(cube_front[0]));
    vertices[3].assign(cube_back, cube_back + sizeof(cube_back) / sizeof(cube_back[0]));
    vertices[4].assign(cube_left, cube_left + sizeof(cube_left) / sizeof(cube_left[0]));
    vertices[5].assign(cube_right, cube_right + sizeof(cube_right) / sizeof(cube_right[0]));
    vertices[6].assign(cube_top, cube_top + sizeof(cube_top) / sizeof(cube_top[0]));
    vertices[7].assign(cube_bottom, cube_bottom + sizeof(cube_bottom) / sizeof(cube_bottom[0]));


    // upload the model to the GPU (explanations omitted for brevity)
    for (int i = 0; i < NUM_MODELS; i++) {

        glGenVertexArrays(1, &vao[i]);
        glGenBuffers(1, &vbo[i]);
        glBindVertexArray(vao[i]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
        glBufferData(GL_ARRAY_BUFFER, vertices[i].size() * sizeof(float), vertices[i].data(), GL_STATIC_DRAW);

        // on the VAO, register the current VBO with the following vertex attribute layout:
        // - the stride length of the vertex array is 11 floats (11 * sizeof(float))
        // - layout location 0 (position) is 3 floats and starts at the first float of the vertex array (offset 0)
        // - layout location 1 (color) is 3 floats and starts at the fourth float (offset 3 * sizeof(float))
        // - layout location 2 (texcoord) is 2 floats and starts at the seventh float (offset 6 * sizeof(float))
        // - layout location 3 (normal) is 3 floats and starts at the ninth float (offset 8 * sizeof(float))
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*) 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*) (3 * sizeof(float)));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*) (6 * sizeof(float)));
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*) (8 * sizeof(float)));

        // enable the layout locations so they can be used by the vertex shader
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
    }

    

    // load our shader program
    shader = gdevLoadShader("ex2.vs", "ex2.fs");
    if (! shader)
        return false;

    // load our texture
    texture[0] = gdevLoadTexture("goldcoin.jpg", GL_REPEAT, true, true); // https://www.freepik.com/free-photo/wood-close-up_969753.htm#fromView=keyword&page=1&position=35&uuid=e18b3cb0-8a52-497b-a8ea-bf73ce8d8fa9&query=Gold+texture
    texture[1] = gdevLoadTexture("wood_texture.jpg", GL_REPEAT, true, true); // https://www.hiclipart.com/free-transparent-background-png-clipart-labui
    
    // Load brick texture for all 6 cube faces
    GLuint brick_texture = gdevLoadTexture("brickwalltexture.jpg", GL_REPEAT, true, true);
    for (int i = 2; i < NUM_MODELS; i++) {
        texture[i] = brick_texture;
    } // https://www.freepik.com/free-photo/brick-wall-background-texture_34862131.htm#fromView=keyword&page=1&position=5&uuid=e7c70f0f-9578-48b7-9546-1bd42e57d650&query=Dungeon+wall+pattern
    
    for (GLuint t: texture) {
        if (! t)
            return false;
    }
    

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    // glFrontFace(GL_CCW);

    // enable OpenGL blending so that texels with alpha values less than one are drawn transparent
    // (you can omit these lines if you don't use alpha)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- Lighting: ensure OpenGL state is ready for lighting (depth test already enabled above) ---
    // (Uniforms for lighting will be set per-frame in render())
    // If you want to set any static lighting state, do it here.

    return true;
}

// called by the main function to do rendering per frame
void render()
{
    // find the elapsed time since the last frame
    double currentTime = glfwGetTime();
    double elapsedTime = (currentTime - previousTime);
    float camSpeed = 5.0f; // units per second
    float moveSpeed = elapsedTime * camSpeed;
    float turnSpeed = elapsedTime * (camSpeed/2);
    previousTime = currentTime;

    // Update animation - move coin along the path
    coinPathTime += elapsedTime / coinPathDuration;
    if (coinPathTime >= 1.0f) {
        coinPathTime -= 1.0f;
        coinPathSegment = (coinPathSegment + 1) % 3;  // cycle through segments
    }
    coinPos = getCoinPositionOnPath(coinPathTime, coinPathSegment);

    // Update animation - move chest along the path independently
    chestPathTime += elapsedTime / chestPathDuration;
    if (chestPathTime >= 1.0f) {
        chestPathTime -= 1.0f;
        chestPathSegment = (chestPathSegment + 1) % 3;  // cycle through segments
    }
    chestPos = getChestPositionOnPath(chestPathTime, chestPathSegment);

    // Camera movement controls (WASD) - relative to camera forward direction
    // Calculate forward and right vectors from current yaw and pitch
    glm::vec3 cameraForward = glm::normalize(glm::vec3(
        sin(yaw),
        0.0f,  // don't move up/down based on pitch
        -cos(yaw)
    ));
    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraForward, glm::vec3(0.0f, 1.0f, 0.0f)));

    // Camera movement is handled in processInput() for WASD and mouse look


    // --- Lighting controls (from box.cpp) ---
    // Point light movement (T/G/F/H/R/Y)
    if (glfwGetKey(pWindow, GLFW_KEY_T) == GLFW_PRESS)
        lightPosition.x += 0.1f;
    if (glfwGetKey(pWindow, GLFW_KEY_G) == GLFW_PRESS)
        lightPosition.x -= 0.1f;
    if (glfwGetKey(pWindow, GLFW_KEY_H) == GLFW_PRESS)
        lightPosition.z += 0.1f;
    if (glfwGetKey(pWindow, GLFW_KEY_F) == GLFW_PRESS)
        lightPosition.z -= 0.1f;
    if (glfwGetKey(pWindow, GLFW_KEY_R) == GLFW_PRESS)
        lightHeight += 0.05f;
    if (glfwGetKey(pWindow, GLFW_KEY_Y) == GLFW_PRESS)
        lightHeight -= 0.05f;

    // Spotlight movement (I/K/J/L/U/O)
    if (glfwGetKey(pWindow, GLFW_KEY_I) == GLFW_PRESS)
        spotX += 0.1f;
    if (glfwGetKey(pWindow, GLFW_KEY_K) == GLFW_PRESS)
        spotX -= 0.1f;
    if (glfwGetKey(pWindow, GLFW_KEY_L) == GLFW_PRESS)
        spotZ += 0.1f;
    if (glfwGetKey(pWindow, GLFW_KEY_J) == GLFW_PRESS)
        spotZ -= 0.1f;
    if (glfwGetKey(pWindow, GLFW_KEY_U) == GLFW_PRESS)
        spotPosition.y += 0.1f;
    if (glfwGetKey(pWindow, GLFW_KEY_O) == GLFW_PRESS)
        spotPosition.y -= 0.1f;

    // Light color and height controls
    if (glfwGetKey(pWindow, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
        lightColor -= glm::vec3(0.1f);
    if (glfwGetKey(pWindow, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
        lightColor += glm::vec3(0.1f);
    if (glfwGetKey(pWindow, GLFW_KEY_EQUAL) == GLFW_PRESS)
        lightHeight += 0.05f;
    if (glfwGetKey(pWindow, GLFW_KEY_MINUS) == GLFW_PRESS)
        lightHeight -= 0.05f;

    float cameraBoundX = 28.0f;
    float cameraBoundZ = 17.0f;
    if (cameraPos.x > cameraBoundX) cameraPos.x = cameraBoundX;
    if (cameraPos.x < -cameraBoundX) cameraPos.x = -cameraBoundX;
    if (cameraPos.z > cameraBoundZ) cameraPos.z = cameraBoundZ;
    if (cameraPos.z < -cameraBoundZ) cameraPos.z = -cameraBoundZ;

    // Camera turning is handled by mouse_callback; arrow keys are not used for camera

    glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glUseProgram(shader);

    // --- Lighting uniforms (set before drawing) ---
    // Update light and spot positions if animated
    lightPosition.y = lightHeight;
    spotPosition = glm::vec3(spotX, spotPosition.y, spotZ);

    glUniform3fv(glGetUniformLocation(shader, "lightPosition"), 1, &lightPosition[0]);
    glUniform3fv(glGetUniformLocation(shader, "lightColor"), 1, &lightColor[0]);
    glUniform1f(glGetUniformLocation(shader, "specColor"), specularity);

    glUniform3fv(glGetUniformLocation(shader, "spotPosition"), 1, &spotPosition[0]);
    glUniform3fv(glGetUniformLocation(shader, "spotDirection"), 1, &spotDirection[0]);
    glUniform1f(glGetUniformLocation(shader, "spotCutoff"), glm::cos(glm::radians(15.0f)));
    glUniform3f(glGetUniformLocation(shader, "spotColor"), 1.0f, 1.0f, 1.0f);

    glUniform3fv(glGetUniformLocation(shader, "cameraPos"), 1, &cameraPos[0]);

    // ... set up the projection matrix...
    glm::mat4 projectionTransform;
    projectionTransform = glm::perspective(glm::radians(45.0f),
                                           (float) WINDOW_WIDTH / WINDOW_HEIGHT,
                                           0.1f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projectionTransform"),
                       1, GL_FALSE, glm::value_ptr(projectionTransform));

    // View
    glm::mat4 view = glm::lookAt(
        cameraPos,
        cameraPos + cameraFront,
        cameraUp);
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewTransform"),
                       1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 viewTransform;
    // Calculate look direction from yaw and pitch
    glm::vec3 lookDirection = glm::vec3(
        sin(yaw) * cos(pitch),
        sin(pitch),
        -cos(yaw) * cos(pitch)
    );

    // Old Cam View Transform (before implementing mouse look)
    // viewTransform = glm::lookAt(
    //     glm::vec3(x, y, z),
    //     glm::vec3(x, y, z) + lookDirection,
    //     glm::vec3(0.0f, 1.0f, 0.0f)
    // );
    // glUniformMatrix4fv(glGetUniformLocation(shader, "viewTransform"),
    //                    1, GL_FALSE, glm::value_ptr(viewTransform));

    // Draw coin at its animated position
    glm::vec3 nextPointCoin = getNextWaypoint(coinPathSegment);
    glm::mat4 coinRotation = getLookAtRotation(coinPos, nextPointCoin);
    float coinBob = sin(currentTime * 3.0f) * 1.50f; // uses vertical bobbing to emulate floating movement
    glm::mat4 modelTransform = glm::translate(glm::mat4(1.0f), coinPos + glm::vec3(0.0f, coinBob, 0.0f));
    modelTransform = modelTransform * coinRotation;
    glUniformMatrix4fv(glGetUniformLocation(shader, "modelTransform"),
                       1, GL_FALSE, glm::value_ptr(modelTransform));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glBindVertexArray(vao[0]);
    glDrawArrays(GL_TRIANGLES, 0, vertices[0].size() / 11);

    // Draw chest at its animated position
    glm::vec3 nextPointChest = getNextWaypoint(chestPathSegment);
    glm::mat4 chestRotation = getLookAtRotation(chestPos, nextPointChest);
    chestRotation = chestRotation * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    float chestBreath = 1.0f + sin(currentTime * 4.0f) * 0.1f; // uses scaling to make the chest look like it's bounding after the coin
    modelTransform = glm::translate(glm::mat4(1.0f), chestPos);
    modelTransform = modelTransform * chestRotation;
    modelTransform = modelTransform * glm::scale(glm::mat4(1.0f), glm::vec3(chestBreath, chestBreath, chestBreath));
    glUniformMatrix4fv(glGetUniformLocation(shader, "modelTransform"),
                       1, GL_FALSE, glm::value_ptr(modelTransform));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glBindVertexArray(vao[1]);
    glDrawArrays(GL_TRIANGLES, 0, vertices[1].size() / 11);

    // Draw the 6 cube faces (stationary, identity transform)
    modelTransform = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader, "modelTransform"),
                       1, GL_FALSE, glm::value_ptr(modelTransform));
    for (int i = 2; i < NUM_MODELS; i++) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture[i]);
        glBindVertexArray(vao[i]);
        glDrawArrays(GL_TRIANGLES, 0, vertices[i].size() / 11);
    }
}

void processInput(GLFWwindow *window)
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;  
    const float cameraSpeed = 15.0f * deltaTime; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
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
/*****************************************************************************/

// handler called by GLFW when there is a keyboard event
void handleKeys(GLFWwindow* pWindow, int key, int scancode, int action, int mode)
{
    // pressing Esc closes the window
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(pWindow, GL_TRUE);
}

// handler called by GLFW when the window is resized
void handleResize(GLFWwindow* pWindow, int width, int height)
{
    // tell OpenGL to do its drawing within the entire "client area" (area within the borders) of the window
    glViewport(0, 0, width, height);
}

// main function
int main(int argc, char** argv)
{
    // initialize GLFW and ask for OpenGL 3.3 core
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // create a GLFW window with the specified width, height, and title
    pWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (! pWindow)
    {
        // gracefully terminate if we cannot create the window
        std::cout << "Cannot create the GLFW window.\n";
        glfwTerminate();
        return -1;
    }

    // make the window the current context of subsequent OpenGL commands,
    // and enable vertical sync and aspect-ratio correction on the GLFW window
    glfwMakeContextCurrent(pWindow);
    glfwSwapInterval(1);
    glfwSetWindowAspectRatio(pWindow, WINDOW_WIDTH, WINDOW_HEIGHT);

    // set up callback functions to handle window system events
    glfwSetKeyCallback(pWindow, handleKeys);
    glfwSetCursorPosCallback(pWindow, mouse_callback);
    glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(pWindow, handleResize);

    // don't miss any momentary keypresses
    glfwSetInputMode(pWindow, GLFW_STICKY_KEYS, GLFW_TRUE);

    // initialize GLAD, which acts as a library loader for the current OS's native OpenGL library
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // if our initial setup is successful...
    if (setup())
    {
        // do rendering in a loop until the user closes the window
        while (! glfwWindowShouldClose(pWindow))
        {
            // Process inputs for player movement
            processInput(pWindow);
            // render our next frame
            // (by default, GLFW uses double-buffering with a front and back buffer;
            // all drawing goes to the back buffer, so the frame does not get shown yet)
            render();

            // swap the GLFW front and back buffers to show the next frame
            glfwSwapBuffers(pWindow);

            // process any window events (such as moving, resizing, keyboard presses, etc.)
            glfwPollEvents();
        }
    }

    // gracefully terminate the program
    glfwTerminate();
    return 0;
}
