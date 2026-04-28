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

#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <gdev.h>

// change this to your desired window attributes
#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE  "Dungeon Scene (WASD move, mouse look, P toggle shadows, N/M softness)"
GLFWwindow *pWindow;

glm::vec3 lightPosition(1.0f, 5.0f, 1.0f);
glm::vec3 lightColor(35.0f, 35.0f, 35.0f);
float specularity = 0.7f;
float lightHeight = 5.0f;

glm::vec3 spotPosition(0.0f, 14.0f, 0.0f);
glm::vec3 spotDirection(0.0f, -1.0f, 0.0f);
float spotX = 0.0f;
float spotZ = 0.0f;

constexpr float spotInnerAngleDegrees = 35.0f;
constexpr float spotOuterAngleDegrees = 45.0f;
constexpr float shadowNearPlane = 0.5f;
constexpr float shadowFarPlane = 120.0f;
constexpr int shadowSoftnessLevels = 5; // Shadow softness levels added to reduce the strain on FPS optionally (N = <, and M = >)
const int shadowSamplesPerAxisByLevel[shadowSoftnessLevels] = {1, 3, 5, 7, 9}; // I have no idea how else to make this more optimized so I just added "settings" for it :(
const float shadowFilterRadiusByLevel[shadowSoftnessLevels] = {0.0f, 1.0f, 1.75f, 2.5f, 3.25f};

constexpr float roomWidth = 60.0f;
constexpr float roomHeight = 30.0f;
constexpr float roomDepth = 37.5f;
constexpr float roomHalfWidth = roomWidth * 0.5f;
constexpr float roomHalfDepth = roomDepth * 0.5f;
constexpr float hallwayWidth = 18.0f;
constexpr float hallwayHalfWidth = hallwayWidth * 0.5f;
constexpr float hallwayLength = 45.0f;
constexpr float hallwayCenterZ = roomHalfDepth + hallwayLength * 0.5f;
constexpr float secondRoomCenterZ = roomDepth + hallwayLength;
constexpr float portalSideWidth = (roomWidth - hallwayWidth) * 0.5f;
constexpr float portalSideOffsetX = hallwayHalfWidth + portalSideWidth * 0.5f;
constexpr float cameraSideMargin = 2.0f;
constexpr float cameraWallMargin = 1.75f;
constexpr float roomCameraHalfWidth = roomHalfWidth - cameraSideMargin;
constexpr float hallwayCameraHalfWidth = hallwayHalfWidth - cameraSideMargin;
constexpr float sceneFarPlane = 160.0f;
constexpr float pointShadowNearPlane = shadowNearPlane;
constexpr float pointShadowFarPlane = sceneFarPlane;
constexpr int pointLightCount = 2;

glm::vec3 secondaryLightPosition(0.0f, 5.0f, secondRoomCenterZ);
glm::vec3 secondaryLightColor(28.0f, 24.0f, 16.0f);

bool shadowsEnabled = true;
int shadowSoftnessLevel = 2;

// Cube faces
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


constexpr int modelCoin = 0;
constexpr int modelChest = 1;
constexpr int modelRoomFront = 2;
constexpr int modelRoomBack = 3;
constexpr int modelRoomLeft = 4;
constexpr int modelRoomRight = 5;
constexpr int modelRoomTop = 6;
constexpr int modelRoomBottom = 7;
constexpr int modelBottle = 8;
constexpr int modelLantern = 9;

const int NUM_MODELS = 10;
std::vector<float> vertices[NUM_MODELS];
int vertexStrides[NUM_MODELS];

// define OpenGL object IDs to represent the vertex array, shader program, and texture in the GPU
GLuint vao[NUM_MODELS];         // vertex array object (stores the render state for our vertex array)
GLuint vbo[NUM_MODELS];         // vertex buffer object (reserves GPU memory for our vertex array)
GLuint shader;      // combined vertex and fragment shader
GLuint texture[NUM_MODELS];     // texture object
GLuint specularMapTexture[NUM_MODELS] = {};
GLuint normalMapTexture[NUM_MODELS] = {};

bool useCoinSpecular = true;
bool useChestSpecular = true;
bool useCubeSpecular = true;
bool useNormalMapping = true;

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
double lastAnimationUpdate = 0.0;

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

constexpr int glassBottleCount = 3;
const glm::vec3 glassBottlePositions[glassBottleCount] = {
    glm::vec3(-23.0f, -14.95f, 8.0f),
    glm::vec3(6.5f, -14.95f, 39.0f),
    glm::vec3(18.0f, -14.95f, 78.0f),
};
const float glassBottleRotations[glassBottleCount] = {18.0f, -32.0f, 24.0f};

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

void fixCoinSurfaceNormals(std::vector<float>& verts)
{
    const int stride = 11;
    const float surfaceThreshold = 0.001f;
    const float flipThreshold = 0.2f;

    if (verts.size() % (stride * 3) != 0)
        return;

    for (std::size_t base = 0; base < verts.size(); base += stride * 3)
    {
        float avgZ = 0.0f;
        float avgNz = 0.0f;
        for (int vertex = 0; vertex < 3; ++vertex)
        {
            std::size_t offset = base + static_cast<std::size_t>(vertex) * stride;
            avgZ += verts[offset + 2];
            avgNz += verts[offset + 5];
        }

        avgZ /= 3.0f;
        avgNz /= 3.0f;

        bool flipFront = avgZ > surfaceThreshold && avgNz < -flipThreshold;
        bool flipBack = avgZ < -surfaceThreshold && avgNz > flipThreshold;
        if (!flipFront && !flipBack)
            continue;

        for (int vertex = 0; vertex < 3; ++vertex)
        {
            std::size_t offset = base + static_cast<std::size_t>(vertex) * stride;
            verts[offset + 3] = -verts[offset + 3];
            verts[offset + 4] = -verts[offset + 4];
            verts[offset + 5] = -verts[offset + 5];
        }
    }
}

bool fileExists(const char* filename)
{
    std::ifstream file(filename);
    return file.good();
}

GLuint loadOptionalTexture(const char* filename)
{
    if (!fileExists(filename))
        return 0;

    return gdevLoadTexture(filename, GL_REPEAT, true, true);
}

float clampValue(float value, float minimum, float maximum)
{
    if (value < minimum)
        return minimum;
    if (value > maximum)
        return maximum;
    return value;
}

float smoothStepValue(float edge0, float edge1, float value)
{
    float t = clampValue((value - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float wrap01(float value)
{
    return value - std::floor(value);
}

GLuint createTextureFromRgbData(const std::vector<unsigned char>& pixels, int width, int height)
{
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    return tex;
}

GLuint createGrayscaleTexture(int size, unsigned char value)
{
    std::vector<unsigned char> pixels(size * size * 3, value);
    return createTextureFromRgbData(pixels, size, size);
}

void appendVertex(std::vector<float>& verts,
                  const glm::vec3& position,
                  const glm::vec3& color,
                  const glm::vec2& texCoord,
                  const glm::vec3& normal)
{
    verts.push_back(position.x);
    verts.push_back(position.y);
    verts.push_back(position.z);
    verts.push_back(color.r);
    verts.push_back(color.g);
    verts.push_back(color.b);
    verts.push_back(texCoord.x);
    verts.push_back(texCoord.y);
    verts.push_back(normal.x);
    verts.push_back(normal.y);
    verts.push_back(normal.z);
}

void appendQuad(std::vector<float>& verts,
                const glm::vec3& bottomLeft,
                const glm::vec3& bottomRight,
                const glm::vec3& topRight,
                const glm::vec3& topLeft,
                const glm::vec3& color,
                const glm::vec3& normal)
{
    appendVertex(verts, bottomLeft, color, glm::vec2(0.0f, 0.0f), normal);
    appendVertex(verts, bottomRight, color, glm::vec2(1.0f, 0.0f), normal);
    appendVertex(verts, topRight, color, glm::vec2(1.0f, 1.0f), normal);

    appendVertex(verts, bottomLeft, color, glm::vec2(0.0f, 0.0f), normal);
    appendVertex(verts, topRight, color, glm::vec2(1.0f, 1.0f), normal);
    appendVertex(verts, topLeft, color, glm::vec2(0.0f, 1.0f), normal);
}

// Helper function that allows me to define a box by giving min and max corner coordinates
// Made this so I could make the bottle and lantern models without needing to use an external modeling program because that was a product of Thristan's genius that I couldn't utilize properly
void appendBox(std::vector<float>& verts,
               const glm::vec3& minCorner,
               const glm::vec3& maxCorner,
               const glm::vec3& color)
{
    appendQuad(verts,
               glm::vec3(minCorner.x, minCorner.y, maxCorner.z),
               glm::vec3(maxCorner.x, minCorner.y, maxCorner.z),
               glm::vec3(maxCorner.x, maxCorner.y, maxCorner.z),
               glm::vec3(minCorner.x, maxCorner.y, maxCorner.z),
               color,
               glm::vec3(0.0f, 0.0f, 1.0f));

    appendQuad(verts,
               glm::vec3(maxCorner.x, minCorner.y, minCorner.z),
               glm::vec3(minCorner.x, minCorner.y, minCorner.z),
               glm::vec3(minCorner.x, maxCorner.y, minCorner.z),
               glm::vec3(maxCorner.x, maxCorner.y, minCorner.z),
               color,
               glm::vec3(0.0f, 0.0f, -1.0f));

    appendQuad(verts,
               glm::vec3(minCorner.x, minCorner.y, minCorner.z),
               glm::vec3(minCorner.x, minCorner.y, maxCorner.z),
               glm::vec3(minCorner.x, maxCorner.y, maxCorner.z),
               glm::vec3(minCorner.x, maxCorner.y, minCorner.z),
               color,
               glm::vec3(-1.0f, 0.0f, 0.0f));

    appendQuad(verts,
               glm::vec3(maxCorner.x, minCorner.y, maxCorner.z),
               glm::vec3(maxCorner.x, minCorner.y, minCorner.z),
               glm::vec3(maxCorner.x, maxCorner.y, minCorner.z),
               glm::vec3(maxCorner.x, maxCorner.y, maxCorner.z),
               color,
               glm::vec3(1.0f, 0.0f, 0.0f));

    appendQuad(verts,
               glm::vec3(minCorner.x, maxCorner.y, maxCorner.z),
               glm::vec3(maxCorner.x, maxCorner.y, maxCorner.z),
               glm::vec3(maxCorner.x, maxCorner.y, minCorner.z),
               glm::vec3(minCorner.x, maxCorner.y, minCorner.z),
               color,
               glm::vec3(0.0f, 1.0f, 0.0f));

    appendQuad(verts,
               glm::vec3(minCorner.x, minCorner.y, minCorner.z),
               glm::vec3(maxCorner.x, minCorner.y, minCorner.z),
               glm::vec3(maxCorner.x, minCorner.y, maxCorner.z),
               glm::vec3(minCorner.x, minCorner.y, maxCorner.z),
               color,
               glm::vec3(0.0f, -1.0f, 0.0f));
}

std::vector<float> createGlassBottleVertices()
{
    std::vector<float> verts;
    glm::vec3 bottleColor(1.0f, 1.0f, 1.0f);

    appendBox(verts, glm::vec3(-0.80f, 0.0f, -0.80f), glm::vec3(0.80f, 3.80f, 0.80f), bottleColor);
    appendBox(verts, glm::vec3(-0.95f, 3.84f, -0.95f), glm::vec3(0.95f, 4.18f, 0.95f), bottleColor);
    appendBox(verts, glm::vec3(-0.35f, 4.22f, -0.35f), glm::vec3(0.35f, 5.35f, 0.35f), bottleColor);
    appendBox(verts, glm::vec3(-0.48f, 5.39f, -0.48f), glm::vec3(0.48f, 5.62f, 0.48f), bottleColor);

    return verts;
}

std::vector<float> createLanternVertices() // Use a bunch of boxes to make a cute simple lantern (I'm really proud of this one it took a lot of time hehe)
{
    std::vector<float> verts;
    glm::vec3 lanternColor(1.0f, 1.0f, 1.0f);

    appendBox(verts, glm::vec3(-0.65f, -1.20f, -0.35f), glm::vec3(0.65f, 1.20f, 0.35f), lanternColor);
    appendBox(verts, glm::vec3(-0.85f, 1.24f, -0.55f), glm::vec3(0.85f, 1.52f, 0.55f), lanternColor);
    appendBox(verts, glm::vec3(-0.85f, -1.52f, -0.55f), glm::vec3(0.85f, -1.24f, 0.55f), lanternColor);

    return verts;
}

GLuint createLanternTexture(int size)
{
    std::vector<unsigned char> pixels(size * size * 3);

    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(size);
            float v = (static_cast<float>(y) + 0.5f) / static_cast<float>(size);

            bool frame = u < 0.18f || u > 0.82f || v < 0.08f || v > 0.92f;
            bool centerBrace = v > 0.46f && v < 0.54f;
            float dx = u - 0.5f;
            float dy = v - 0.5f;
            float glow = clampValue(1.0f - std::sqrt(dx * dx + dy * dy) * 1.85f, 0.0f, 1.0f);

            glm::vec3 frameColor(0.23f, 0.14f, 0.05f);
            glm::vec3 panelColor(0.72f + 0.28f * glow,
                                 0.52f + 0.25f * glow,
                                 0.16f + 0.16f * glow);
            glm::vec3 color = (frame || centerBrace) ? frameColor : panelColor;

            std::size_t index = static_cast<std::size_t>(y * size + x) * 3;
            pixels[index + 0] = static_cast<unsigned char>(clampValue(color.r, 0.0f, 1.0f) * 255.0f);
            pixels[index + 1] = static_cast<unsigned char>(clampValue(color.g, 0.0f, 1.0f) * 255.0f);
            pixels[index + 2] = static_cast<unsigned char>(clampValue(color.b, 0.0f, 1.0f) * 255.0f);
        }
    }

    return createTextureFromRgbData(pixels, size, size);
}

float sampleWoodHeight(float u, float v)
{
    float grain = 0.5f + 0.5f * std::sin((u * 48.0f) + 4.0f * std::sin(v * 14.0f));
    float rings = 0.5f + 0.5f * std::sin((u + 0.15f * std::sin(v * 6.0f)) * 20.0f);
    return 0.65f * grain + 0.35f * rings;
}

float sampleBrickHeight(float u, float v)
{
    float scaledU = u * 4.0f;
    float scaledV = v * 4.0f;
    float row = std::floor(scaledV);
    float rowOffset = std::fmod(row, 2.0f) >= 1.0f ? 0.5f : 0.0f;

    float localU = wrap01(scaledU + rowOffset);
    float localV = wrap01(scaledV);
    float edgeU = std::min(localU, 1.0f - localU);
    float edgeV = std::min(localV, 1.0f - localV);

    float brickMask = smoothStepValue(0.05f, 0.12f, edgeU)
                    * smoothStepValue(0.05f, 0.12f, edgeV);
    float brickNoise = 0.5f + 0.5f * std::sin(u * 26.0f + v * 19.0f);
    return 0.12f + brickMask * (0.78f + 0.10f * brickNoise);
}

float sampleWoodSpecular(float u, float v)
{
    return 0.18f + 0.35f * sampleWoodHeight(u, v);
}

float sampleBrickSpecular(float u, float v)
{
    return 0.10f + 0.28f * sampleBrickHeight(u, v);
}

GLuint createNormalMapTexture(int size, float (*heightSampler)(float, float), float strength)
{
    std::vector<unsigned char> pixels(size * size * 3);
    float delta = 1.0f / static_cast<float>(size);

    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(size);
            float v = (static_cast<float>(y) + 0.5f) / static_cast<float>(size);

            float hL = heightSampler(wrap01(u - delta), v);
            float hR = heightSampler(wrap01(u + delta), v);
            float hD = heightSampler(u, wrap01(v - delta));
            float hU = heightSampler(u, wrap01(v + delta));

            glm::vec3 normal = glm::normalize(glm::vec3(
                (hL - hR) * strength,
                (hD - hU) * strength,
                1.0f));

            std::size_t index = static_cast<std::size_t>(y * size + x) * 3;
            pixels[index + 0] = static_cast<unsigned char>(clampValue(normal.x * 0.5f + 0.5f, 0.0f, 1.0f) * 255.0f);
            pixels[index + 1] = static_cast<unsigned char>(clampValue(normal.y * 0.5f + 0.5f, 0.0f, 1.0f) * 255.0f);
            pixels[index + 2] = static_cast<unsigned char>(clampValue(normal.z * 0.5f + 0.5f, 0.0f, 1.0f) * 255.0f);
        }
    }

    return createTextureFromRgbData(pixels, size, size);
}

GLuint createSpecularMapTexture(int size, float (*specularSampler)(float, float))
{
    std::vector<unsigned char> pixels(size * size * 3);

    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(size);
            float v = (static_cast<float>(y) + 0.5f) / static_cast<float>(size);
            unsigned char intensity = static_cast<unsigned char>(clampValue(specularSampler(u, v), 0.0f, 1.0f) * 255.0f);

            std::size_t index = static_cast<std::size_t>(y * size + x) * 3;
            pixels[index + 0] = intensity;
            pixels[index + 1] = intensity;
            pixels[index + 2] = intensity;
        }
    }

    return createTextureFromRgbData(pixels, size, size);
}

void addTangentsToVertices(std::vector<float>& verts)
{
    const int oldStride = 11;
    if (verts.size() % oldStride != 0)
        return;

    std::size_t vertexCount = verts.size() / oldStride;
    std::vector<glm::vec3> tangents(vertexCount, glm::vec3(0.0f));

    for (std::size_t i = 0; i + 2 < vertexCount; i += 3)
    {
        std::size_t i0 = i * oldStride;
        std::size_t i1 = (i + 1) * oldStride;
        std::size_t i2 = (i + 2) * oldStride;

        glm::vec3 p0(verts[i0 + 0], verts[i0 + 1], verts[i0 + 2]);
        glm::vec3 p1(verts[i1 + 0], verts[i1 + 1], verts[i1 + 2]);
        glm::vec3 p2(verts[i2 + 0], verts[i2 + 1], verts[i2 + 2]);

        glm::vec2 uv0(verts[i0 + 6], verts[i0 + 7]);
        glm::vec2 uv1(verts[i1 + 6], verts[i1 + 7]);
        glm::vec2 uv2(verts[i2 + 6], verts[i2 + 7]);

        glm::vec3 edge1 = p1 - p0;
        glm::vec3 edge2 = p2 - p0;
        glm::vec2 dUV1 = uv1 - uv0;
        glm::vec2 dUV2 = uv2 - uv0;

        float determinant = dUV1.x * dUV2.y - dUV2.x * dUV1.y;
        if (std::abs(determinant) < 1e-6f)
            continue;

        float factor = 1.0f / determinant;
        glm::vec3 tangent = factor * (dUV2.y * edge1 - dUV1.y * edge2);

        tangents[i + 0] += tangent;
        tangents[i + 1] += tangent;
        tangents[i + 2] += tangent;
    }

    for (std::size_t vertex = 0; vertex < vertexCount; ++vertex)
    {
        glm::vec3 tangent = tangents[vertex];
        if (glm::dot(tangent, tangent) > 1e-6f)
            tangents[vertex] = glm::normalize(tangent);
        else
            tangents[vertex] = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    const int newStride = 14;
    std::vector<float> result;
    result.reserve(vertexCount * newStride);

    for (std::size_t vertex = 0; vertex < vertexCount; ++vertex)
    {
        std::size_t base = vertex * oldStride;
        for (int element = 0; element < oldStride; ++element)
            result.push_back(verts[base + element]);

        glm::vec3 tangent = tangents[vertex];
        result.push_back(tangent.x);
        result.push_back(tangent.y);
        result.push_back(tangent.z);
    }

    verts.swap(result);
}

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

    void updateSceneAnimation(double elapsedTime)
    {
        coinPathTime += static_cast<float>(elapsedTime / coinPathDuration);
        if (coinPathTime >= 1.0f) {
            coinPathTime -= 1.0f;
            coinPathSegment = (coinPathSegment + 1) % 3;
        }
        coinPos = getCoinPositionOnPath(coinPathTime, coinPathSegment);

        chestPathTime += static_cast<float>(elapsedTime / chestPathDuration);
        if (chestPathTime >= 1.0f) {
            chestPathTime -= 1.0f;
            chestPathSegment = (chestPathSegment + 1) % 3;
        }
        chestPos = getChestPositionOnPath(chestPathTime, chestPathSegment);
    }

    glm::mat4 getCoinModelTransform(double currentTime)
    {
        glm::vec3 nextPointCoin = getNextWaypoint(coinPathSegment);
        glm::mat4 coinRotation = getLookAtRotation(coinPos, nextPointCoin);
        float coinBob = std::sin(static_cast<float>(currentTime) * 3.0f) * 1.50f;

        glm::mat4 modelTransform = glm::translate(glm::mat4(1.0f), coinPos + glm::vec3(0.0f, coinBob, 0.0f));
        return modelTransform * coinRotation;
    }

    glm::mat4 getChestModelTransform(double currentTime)
    {
        glm::vec3 nextPointChest = getNextWaypoint(chestPathSegment);
        glm::mat4 chestRotation = getLookAtRotation(chestPos, nextPointChest);
        chestRotation = chestRotation * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        float chestBreath = 1.0f + std::sin(static_cast<float>(currentTime) * 4.0f) * 0.1f;
        glm::mat4 modelTransform = glm::translate(glm::mat4(1.0f), chestPos);
        modelTransform = modelTransform * chestRotation;
        return modelTransform * glm::scale(glm::mat4(1.0f), glm::vec3(chestBreath, chestBreath, chestBreath));
    }

    glm::mat4 getGlassBottleModelTransform(int bottleIndex)
    {
        glm::mat4 modelTransform = glm::translate(glm::mat4(1.0f), glassBottlePositions[bottleIndex]);
        return modelTransform * glm::rotate(glm::mat4(1.0f), glm::radians(glassBottleRotations[bottleIndex]), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::mat4 getLanternModelTransform(const glm::vec3& lanternPosition)
    {
        return glm::translate(glm::mat4(1.0f), lanternPosition);
    }

    glm::vec3 getSpotlightUpVector()
    {
        glm::vec3 forward = glm::normalize(spotDirection);
        if (std::abs(glm::dot(forward, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f)
            return glm::vec3(0.0f, 0.0f, 1.0f);

        return glm::vec3(0.0f, 1.0f, 0.0f);
    }

    glm::mat4 getSpotlightTransform()
    {
        glm::mat4 lightProjection = glm::perspective(
            glm::radians(spotOuterAngleDegrees * 2.0f),
            1.0f,
            shadowNearPlane,
            shadowFarPlane);

        glm::mat4 lightView = glm::lookAt(
            spotPosition,
            spotPosition + glm::normalize(spotDirection),
            getSpotlightUpVector());

        return lightProjection * lightView;
    }

    void getPointLightTransforms(const glm::vec3& pointLightPosition, glm::mat4 transforms[6]) // Ensure that the point light projects shadows in every direction
    {
        glm::mat4 lightProjection = glm::perspective(
            glm::radians(90.0f),
            1.0f,
            pointShadowNearPlane,
            pointShadowFarPlane);

        transforms[0] = lightProjection * glm::lookAt(pointLightPosition, pointLightPosition + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        transforms[1] = lightProjection * glm::lookAt(pointLightPosition, pointLightPosition + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        transforms[2] = lightProjection * glm::lookAt(pointLightPosition, pointLightPosition + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        transforms[3] = lightProjection * glm::lookAt(pointLightPosition, pointLightPosition + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        transforms[4] = lightProjection * glm::lookAt(pointLightPosition, pointLightPosition + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        transforms[5] = lightProjection * glm::lookAt(pointLightPosition, pointLightPosition + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    }

    void drawModel(GLuint activeShader, int modelIndex, const glm::mat4& modelTransform, bool bindTexture)
    {
        glUniformMatrix4fv(glGetUniformLocation(activeShader, "modelTransform"),
                           1, GL_FALSE, glm::value_ptr(modelTransform));

        GLint normalDirectionLocation = glGetUniformLocation(activeShader, "normalDirection");
        if (normalDirectionLocation != -1)
        {
            float normalDirection = (modelIndex >= modelRoomFront && modelIndex <= modelRoomBottom) ? -1.0f : 1.0f;
            glUniform1f(normalDirectionLocation, normalDirection);
        }

        GLint normalTransformLocation = glGetUniformLocation(activeShader, "normalTransform");
        if (normalTransformLocation != -1)
        {
            glm::mat4 normalTransform = glm::transpose(glm::inverse(modelTransform));
            glUniformMatrix4fv(normalTransformLocation,
                               1, GL_FALSE, glm::value_ptr(normalTransform));
        }

        if (bindTexture)
        {
            bool useSpec = false;
            bool useNorm = false;
            float specularStrength = specularity;
            float materialOpacity = 1.0f;
            glm::vec3 materialEmissionColor(0.0f, 0.0f, 0.0f);
            float materialEmissionStrength = 0.0f;

            if (modelIndex == 0)
            {
                useSpec = useCoinSpecular && specularMapTexture[0] != 0;
                specularStrength = useSpec ? 5.0f : specularity;
            }
            else if (modelIndex == 1)
            {
                useSpec = useChestSpecular && specularMapTexture[1] != 0;
                useNorm = useNormalMapping && normalMapTexture[1] != 0;
                specularStrength = useSpec ? 3.0f : specularity;
            }
            else if (modelIndex == modelBottle)
            {
                useSpec = specularMapTexture[modelBottle] != 0;
                useNorm = useNormalMapping && normalMapTexture[modelBottle] != 0;
                specularStrength = useSpec ? 4.0f : 1.6f;
                materialOpacity = 0.34f;
            }
            else if (modelIndex == modelLantern)
            {
                specularStrength = 0.35f;
                materialEmissionColor = glm::vec3(1.0f, 0.78f, 0.30f);
                materialEmissionStrength = 2.6f;
            }
            else
            {
                useSpec = useCubeSpecular && specularMapTexture[modelIndex] != 0;
                useNorm = useNormalMapping && normalMapTexture[modelIndex] != 0;
                specularStrength = useSpec ? 2.0f : specularity;
            }

            GLint specColorLocation = glGetUniformLocation(activeShader, "specColor");
            if (specColorLocation != -1)
                glUniform1f(specColorLocation, specularStrength);

            GLint materialOpacityLocation = glGetUniformLocation(activeShader, "materialOpacity");
            if (materialOpacityLocation != -1)
                glUniform1f(materialOpacityLocation, materialOpacity);

            GLint materialEmissionColorLocation = glGetUniformLocation(activeShader, "materialEmissionColor");
            if (materialEmissionColorLocation != -1)
                glUniform3fv(materialEmissionColorLocation, 1, glm::value_ptr(materialEmissionColor));

            GLint materialEmissionStrengthLocation = glGetUniformLocation(activeShader, "materialEmissionStrength");
            if (materialEmissionStrengthLocation != -1)
                glUniform1f(materialEmissionStrengthLocation, materialEmissionStrength);

            GLint useSpecularTextureLocation = glGetUniformLocation(activeShader, "useSpecularTexture");
            if (useSpecularTextureLocation != -1)
                glUniform1i(useSpecularTextureLocation, useSpec ? 1 : 0);

            GLint useNormalMapLocation = glGetUniformLocation(activeShader, "useNormalMap");
            if (useNormalMapLocation != -1)
                glUniform1i(useNormalMapLocation, useNorm ? 1 : 0);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture[modelIndex]);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, useSpec ? specularMapTexture[modelIndex] : 0);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, useNorm ? normalMapTexture[modelIndex] : 0);
        }

        glBindVertexArray(vao[modelIndex]);
        glDrawArrays(GL_TRIANGLES, 0, vertices[modelIndex].size() / vertexStrides[modelIndex]);
    }

    glm::mat4 makeScaledTransform(const glm::vec3& translation, const glm::vec3& scale)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation);
        return transform * glm::scale(glm::mat4(1.0f), scale);
    }

    void drawPortalPillars(GLuint activeShader, int wallModelIndex, const glm::mat4& baseTransform, bool bindTexture)
    {
        glm::vec3 pillarScale(portalSideWidth / roomWidth, 1.0f, 1.0f);

        drawModel(activeShader,
                  wallModelIndex,
                  baseTransform * makeScaledTransform(glm::vec3(-portalSideOffsetX, 0.0f, 0.0f), pillarScale),
                  bindTexture);
        drawModel(activeShader,
                  wallModelIndex,
                  baseTransform * makeScaledTransform(glm::vec3(portalSideOffsetX, 0.0f, 0.0f), pillarScale),
                  bindTexture);
    }

    void drawRoomShell(GLuint activeShader, const glm::mat4& roomTransform, bool openFront, bool openBack, bool bindTexture)
    {
        if (openFront)
            drawPortalPillars(activeShader, modelRoomFront, roomTransform, bindTexture);
        else
            drawModel(activeShader, modelRoomFront, roomTransform, bindTexture);

        if (openBack)
            drawPortalPillars(activeShader, modelRoomBack, roomTransform, bindTexture);
        else
            drawModel(activeShader, modelRoomBack, roomTransform, bindTexture);

        drawModel(activeShader, modelRoomLeft, roomTransform, bindTexture);
        drawModel(activeShader, modelRoomRight, roomTransform, bindTexture);
        drawModel(activeShader, modelRoomTop, roomTransform, bindTexture);
        drawModel(activeShader, modelRoomBottom, roomTransform, bindTexture);
    }

    void drawPointLightLanterns(GLuint activeShader, bool bindTexture)
    {
        drawModel(activeShader, modelLantern, getLanternModelTransform(lightPosition), bindTexture);
        drawModel(activeShader, modelLantern, getLanternModelTransform(secondaryLightPosition), bindTexture);
    }

    void drawScene(GLuint activeShader, double currentTime, bool bindTexture, bool drawLanterns)
    {
        drawModel(activeShader, modelCoin, getCoinModelTransform(currentTime), bindTexture);
        drawModel(activeShader, modelChest, getChestModelTransform(currentTime), bindTexture);

        glm::mat4 firstRoomTransform(1.0f);
        drawRoomShell(activeShader, firstRoomTransform, true, false, bindTexture);

        glm::mat4 hallwayTransform = makeScaledTransform(
            glm::vec3(0.0f, 0.0f, hallwayCenterZ),
            glm::vec3(hallwayWidth / roomWidth, 1.0f, hallwayLength / roomDepth));
        drawModel(activeShader, modelRoomLeft, hallwayTransform, bindTexture);
        drawModel(activeShader, modelRoomRight, hallwayTransform, bindTexture);
        drawModel(activeShader, modelRoomTop, hallwayTransform, bindTexture);
        drawModel(activeShader, modelRoomBottom, hallwayTransform, bindTexture);

        glm::mat4 secondRoomTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, secondRoomCenterZ));
        drawRoomShell(activeShader, secondRoomTransform, false, true, bindTexture);

        if (drawLanterns)
            drawPointLightLanterns(activeShader, bindTexture);
    }

    void drawGlassBottles(GLuint activeShader, bool bindTexture)
    {
        for (int bottleIndex = 0; bottleIndex < glassBottleCount; ++bottleIndex)
            drawModel(activeShader, modelBottle, getGlassBottleModelTransform(bottleIndex), bindTexture);
    }

///////////////////////////////////////////////////////////////////////////////
// SHADOW MAPPING CODE

#define SHADOW_SIZE 1024
GLuint shadowMapFbo;      // shadow map framebuffer object
GLuint shadowMapTexture;  // shadow map texture
GLuint shadowMapShader;   // shadow map shader
GLuint pointShadowMapFbo;
GLuint pointShadowMapTextures[pointLightCount] = {};
GLuint pointShadowMapShader;

void restoreWindowViewport()
{
    int width, height;
    glfwGetFramebufferSize(pWindow, &width, &height);
    glViewport(0, 0, width, height);
}

bool setupShadowMap()
{
    // create the FBO for rendering shadows
    glGenFramebuffers(1, &shadowMapFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFbo);

    // attach a texture object to the framebuffer
    glGenTextures(1, &shadowMapTexture);
    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_SIZE, SHADOW_SIZE,
                 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // check if we did everything right
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Could not create custom framebuffer.\n";
        return false;
    }

    // load the shader program for drawing the shadow map
    shadowMapShader = gdevLoadShader("demo8s.vs", "demo8s.fs");
    if (! shadowMapShader)
        return false;

    // set the framebuffer back to the default onscreen buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

bool setupPointShadowMap()
{
    glGenFramebuffers(1, &pointShadowMapFbo);
    glGenTextures(pointLightCount, pointShadowMapTextures);

    for (int lightIndex = 0; lightIndex < pointLightCount; ++lightIndex)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, pointShadowMapTextures[lightIndex]);
        for (int face = 0; face < 6; ++face)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                         0,
                         GL_DEPTH_COMPONENT,
                         SHADOW_SIZE,
                         SHADOW_SIZE,
                         0,
                         GL_DEPTH_COMPONENT,
                         GL_FLOAT,
                         NULL);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, pointShadowMapFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                           pointShadowMapTextures[0],
                           0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Could not create point light shadow framebuffer.\n";
        return false;
    }

    pointShadowMapShader = gdevLoadShader("point_shadow.vs", "point_shadow.fs");
    if (! pointShadowMapShader)
        return false;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void renderShadowMap(const glm::mat4& lightTransform, double currentTime)
{
    // use the shadow framebuffer for drawing the shadow map
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFbo);

    // the viewport should be the size of the shadow map
    glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);

    // clear the shadow map
    // (we don't have a color buffer attachment, so no need to clear that)
    glClear(GL_DEPTH_BUFFER_BIT);

    // using the shadow map shader...
    glUseProgram(shadowMapShader);

    glUniformMatrix4fv(glGetUniformLocation(shadowMapShader, "lightTransform"),
                       1, GL_FALSE, glm::value_ptr(lightTransform));
    drawScene(shadowMapShader, currentTime, false, true);
    drawGlassBottles(shadowMapShader, false);
    
    // set the framebuffer back to the default onscreen buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    restoreWindowViewport();

}

void renderPointShadowMap(int lightIndex, const glm::vec3& pointLightPosition, double currentTime)
{
    glm::mat4 lightTransforms[6];
    getPointLightTransforms(pointLightPosition, lightTransforms);

    glBindFramebuffer(GL_FRAMEBUFFER, pointShadowMapFbo);
    glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
    glUseProgram(pointShadowMapShader);

    glUniform3fv(glGetUniformLocation(pointShadowMapShader, "lightPosition"), 1, &pointLightPosition[0]);
    glUniform1f(glGetUniformLocation(pointShadowMapShader, "farPlane"), pointShadowFarPlane);

    for (int face = 0; face < 6; ++face)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                               pointShadowMapTextures[lightIndex],
                               0);
        glClear(GL_DEPTH_BUFFER_BIT);

        glUniformMatrix4fv(glGetUniformLocation(pointShadowMapShader, "lightTransform"),
                           1,
                           GL_FALSE,
                           glm::value_ptr(lightTransforms[face]));

        drawScene(pointShadowMapShader, currentTime, false, false);
        drawGlassBottles(pointShadowMapShader, false);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    restoreWindowViewport();

}

void renderPointShadowMaps(double currentTime)
{
    const glm::vec3 pointLightPositions[pointLightCount] = {lightPosition, secondaryLightPosition};
    for (int lightIndex = 0; lightIndex < pointLightCount; ++lightIndex)
        renderPointShadowMap(lightIndex, pointLightPositions[lightIndex], currentTime);
}

// SHADOW MAPPING CODE
///////////////////////////////////////////////////////////////////////////////



// called by the main function to do initial setup, such as uploading vertex
// arrays, shader programs, etc.; returns true if successful, false otherwise
bool setup()
{
    for (int i = 0; i < NUM_MODELS; ++i)
        vertexStrides[i] = 11;

    load_model("coinarray.txt", vertices[0]);
    fixCoinSurfaceNormals(vertices[0]);
    load_model("chestarray.txt", vertices[1]);
    
    vertices[2].assign(cube_front, cube_front + sizeof(cube_front) / sizeof(cube_front[0]));
    vertices[3].assign(cube_back, cube_back + sizeof(cube_back) / sizeof(cube_back[0]));
    vertices[4].assign(cube_left, cube_left + sizeof(cube_left) / sizeof(cube_left[0]));
    vertices[5].assign(cube_right, cube_right + sizeof(cube_right) / sizeof(cube_right[0]));
    vertices[6].assign(cube_top, cube_top + sizeof(cube_top) / sizeof(cube_top[0]));
    vertices[7].assign(cube_bottom, cube_bottom + sizeof(cube_bottom) / sizeof(cube_bottom[0]));
    vertices[modelBottle] = createGlassBottleVertices();
    vertices[modelLantern] = createLanternVertices();

    addTangentsToVertices(vertices[1]);
    vertexStrides[1] = 14;
    for (int i = 2; i < NUM_MODELS; ++i)
    {
        addTangentsToVertices(vertices[i]);
        vertexStrides[i] = 14;
    }


    for (int i = 0; i < NUM_MODELS; i++) {

        glGenVertexArrays(1, &vao[i]);
        glGenBuffers(1, &vbo[i]);
        glBindVertexArray(vao[i]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
        glBufferData(GL_ARRAY_BUFFER, vertices[i].size() * sizeof(float), vertices[i].data(), GL_STATIC_DRAW);

        int stride = vertexStrides[i];

        // on the VAO, register the current VBO with the following vertex attribute layout:
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) (3 * sizeof(float)));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) (6 * sizeof(float)));
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) (8 * sizeof(float)));

        if (stride == 14)
        {
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) (11 * sizeof(float)));
            glEnableVertexAttribArray(4);
        }

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

    GLuint coinSpecularTexture = gdevLoadTexture("specular_goldcoin.jpg", GL_REPEAT, true, true);
    GLuint chestSpecularTexture = gdevLoadTexture("specular_wood_texture.jpg", GL_REPEAT, true, true);
    GLuint cubeSpecularTexture = gdevLoadTexture("specular_brickwalltexture.jpg", GL_REPEAT, true, true);
    GLuint bottleTexture = gdevLoadTexture("glassbottletexture.jpg", GL_REPEAT, true, true);
    GLuint bottleSpecularTexture = gdevLoadTexture("specular_glassbottletexture.png", GL_REPEAT, true, true);
    GLuint lanternTexture = createLanternTexture(64);

    GLuint chestNormalTexture = gdevLoadTexture("normmap_wood_texture.png", GL_REPEAT, true, true);
    GLuint cubeNormalTexture = gdevLoadTexture("normmap_brickwalltexture.png", GL_REPEAT, true, true);
    GLuint bottleNormalTexture = gdevLoadTexture("normmap_glassbottletexture.png", GL_REPEAT, true, true);

    specularMapTexture[0] = coinSpecularTexture;
    specularMapTexture[1] = chestSpecularTexture;
    normalMapTexture[0] = 0;
    normalMapTexture[1] = chestNormalTexture;
    for (int i = 2; i < NUM_MODELS; ++i)
    {
        specularMapTexture[i] = cubeSpecularTexture;
        normalMapTexture[i] = cubeNormalTexture;
    }
    texture[modelBottle] = bottleTexture;
    specularMapTexture[modelBottle] = bottleSpecularTexture;
    normalMapTexture[modelBottle] = bottleNormalTexture;
    texture[modelLantern] = lanternTexture;
    specularMapTexture[modelLantern] = 0;
    normalMapTexture[modelLantern] = 0;
    
    for (GLuint t: texture) {
        if (! t)
            return false;
    }

    if (!specularMapTexture[0] || !specularMapTexture[1] || !specularMapTexture[2] || !specularMapTexture[modelBottle])
        return false;

    if (!normalMapTexture[1] || !normalMapTexture[2] || !normalMapTexture[modelBottle])
        return false;
    

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    // glFrontFace(GL_CCW);

    // enable OpenGL blending so that texels with alpha values less than one are drawn transparent
    // (you can omit these lines if you don't use alpha)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ///////////////////////////////////////////////////////////////////////////
    // setup shadow rendering
    if (! setupShadowMap())
        return false;
    if (! setupPointShadowMap())
        return false;
    ///////////////////////////////////////////////////////////////////////////

    return true;
}

// called by the main function to do rendering per frame
void render()
{
    static float spotYaw = 0.0f;
    static float spotPitch = -90.0f;
    const float spotTurnSpeed = 1.0f; // degrees per frame
    if (glfwGetKey(pWindow, GLFW_KEY_LEFT) == GLFW_PRESS)
        spotYaw -= spotTurnSpeed;
    if (glfwGetKey(pWindow, GLFW_KEY_RIGHT) == GLFW_PRESS)
        spotYaw += spotTurnSpeed;
    if (glfwGetKey(pWindow, GLFW_KEY_UP) == GLFW_PRESS)
        spotPitch += spotTurnSpeed;
    if (glfwGetKey(pWindow, GLFW_KEY_DOWN) == GLFW_PRESS)
        spotPitch -= spotTurnSpeed;

    

    if (spotPitch > 89.0f) spotPitch = 89.0f;
    if (spotPitch < -89.0f) spotPitch = -89.0f;

    float yawRad = glm::radians(spotYaw);
    float pitchRad = glm::radians(spotPitch);
    spotDirection.x = std::cos(pitchRad) * std::sin(yawRad);
    spotDirection.y = std::sin(pitchRad);
    spotDirection.z = -std::cos(pitchRad) * std::cos(yawRad);
    spotDirection = glm::normalize(spotDirection);

    double currentTime = glfwGetTime();
    if (lastAnimationUpdate == 0.0)
        lastAnimationUpdate = currentTime;

    double elapsedTime = currentTime - lastAnimationUpdate;
    lastAnimationUpdate = currentTime;
    if (elapsedTime < 0.0)
        elapsedTime = 0.0;

    updateSceneAnimation(elapsedTime);
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

    if (glfwGetKey(pWindow, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
        lightColor -= glm::vec3(0.1f);
    if (glfwGetKey(pWindow, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
        lightColor += glm::vec3(0.1f);
    if (glfwGetKey(pWindow, GLFW_KEY_EQUAL) == GLFW_PRESS)
        lightHeight += 0.05f;
    if (glfwGetKey(pWindow, GLFW_KEY_MINUS) == GLFW_PRESS)
        lightHeight -= 0.05f;

    lightPosition.y = lightHeight;
    spotPosition = glm::vec3(spotX, spotPosition.y, spotZ);

    float maxCameraZ = secondRoomCenterZ + roomHalfDepth - cameraWallMargin;
    if (cameraPos.x > roomCameraHalfWidth) cameraPos.x = roomCameraHalfWidth;
    if (cameraPos.x < -roomCameraHalfWidth) cameraPos.x = -roomCameraHalfWidth;
    if (cameraPos.z > maxCameraZ) cameraPos.z = maxCameraZ;
    if (cameraPos.z < -roomHalfDepth + cameraWallMargin) cameraPos.z = -roomHalfDepth + cameraWallMargin;

    if (cameraPos.z > roomHalfDepth && cameraPos.z < roomHalfDepth + hallwayLength)
    {
        if (cameraPos.x > hallwayCameraHalfWidth) cameraPos.x = hallwayCameraHalfWidth;
        if (cameraPos.x < -hallwayCameraHalfWidth) cameraPos.x = -hallwayCameraHalfWidth;
    }

    if (cameraPos.z > roomHalfDepth - cameraWallMargin
        && cameraPos.z < roomHalfDepth + cameraWallMargin
        && std::abs(cameraPos.x) > hallwayCameraHalfWidth)
        cameraPos.z = roomHalfDepth - cameraWallMargin;

    if (cameraPos.z < roomHalfDepth + hallwayLength + cameraWallMargin
        && cameraPos.z > roomHalfDepth + hallwayLength
        && std::abs(cameraPos.x) > hallwayCameraHalfWidth)
    {
        cameraPos.z = roomHalfDepth + hallwayLength + cameraWallMargin;
    }

    glm::mat4 lightTransform(1.0f);
    if (shadowsEnabled)
    {
        renderPointShadowMaps(currentTime);
        lightTransform = getSpotlightTransform();
        renderShadowMap(lightTransform, currentTime);
    }

    glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader);

    glUniform3fv(glGetUniformLocation(shader, "lightPosition"), 1, &lightPosition[0]);
    glUniform3fv(glGetUniformLocation(shader, "lightColor"), 1, &lightColor[0]);
    glUniform3fv(glGetUniformLocation(shader, "secondaryLightPosition"), 1, &secondaryLightPosition[0]);
    glUniform3fv(glGetUniformLocation(shader, "secondaryLightColor"), 1, &secondaryLightColor[0]);
    glUniform1f(glGetUniformLocation(shader, "specColor"), specularity);

    glUniform3fv(glGetUniformLocation(shader, "spotPosition"), 1, &spotPosition[0]);
    glUniform3fv(glGetUniformLocation(shader, "spotDirection"), 1, &spotDirection[0]);
    glUniform1f(glGetUniformLocation(shader, "spotCutoff"), glm::cos(glm::radians(spotInnerAngleDegrees)));
    glUniform1f(glGetUniformLocation(shader, "spotOuterCutoff"), glm::cos(glm::radians(spotOuterAngleDegrees)));
    glUniform3f(glGetUniformLocation(shader, "spotColor"), 1.0f, 1.0f, 1.0f);

    glUniform3fv(glGetUniformLocation(shader, "cameraPos"), 1, &cameraPos[0]);
    glUniform1i(glGetUniformLocation(shader, "shaderTexture"), 0);
    glUniform1i(glGetUniformLocation(shader, "specularTexture"), 1);
    glUniform1i(glGetUniformLocation(shader, "normalMap"), 2);
    glUniform1i(glGetUniformLocation(shader, "shadowMap"), 3);
    glUniform1i(glGetUniformLocation(shader, "pointShadowMap"), 4);
    glUniform1i(glGetUniformLocation(shader, "secondaryPointShadowMap"), 5);
    glUniform1i(glGetUniformLocation(shader, "shadowsEnabled"), shadowsEnabled ? 1 : 0);
    glUniform1i(glGetUniformLocation(shader, "shadowSamplesPerAxis"), shadowSamplesPerAxisByLevel[shadowSoftnessLevel]);
    glUniform1f(glGetUniformLocation(shader, "shadowFilterRadius"), shadowFilterRadiusByLevel[shadowSoftnessLevel]);
    glUniform1f(glGetUniformLocation(shader, "pointShadowFarPlane"), pointShadowFarPlane);

    int width, height;
    glfwGetFramebufferSize(pWindow, &width, &height);
    if (height <= 0)
        height = 1;

    glm::mat4 projectionTransform;
    projectionTransform = glm::perspective(glm::radians(fov),
                                           static_cast<float>(width) / static_cast<float>(height),
                                           0.1f, sceneFarPlane);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projectionTransform"),
                       1, GL_FALSE, glm::value_ptr(projectionTransform));

    // View
    glm::mat4 view = glm::lookAt(
        cameraPos,
        cameraPos + cameraFront,
        cameraUp);
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewTransform"),
                       1, GL_FALSE, glm::value_ptr(view));

    ///////////////////////////////////////////////////////////////////////////
    glUniformMatrix4fv(glGetUniformLocation(shader, "lightTransform"),
                       1, GL_FALSE, glm::value_ptr(lightTransform));

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_CUBE_MAP, pointShadowMapTextures[0]);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, pointShadowMapTextures[1]);
    ///////////////////////////////////////////////////////////////////////////

    drawScene(shader, currentTime, true, true);

    glDepthMask(GL_FALSE);
    drawGlassBottles(shader, true);
    glDepthMask(GL_TRUE);
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
    (void) scancode;
    (void) mode;

    // pressing Esc closes the window
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(pWindow, GL_TRUE);

    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_P)
            shadowsEnabled = !shadowsEnabled;
        else if (key == GLFW_KEY_N && shadowSoftnessLevel > 0)
            shadowSoftnessLevel--;
        else if (key == GLFW_KEY_M && shadowSoftnessLevel < shadowSoftnessLevels - 1)
            shadowSoftnessLevel++;
    }
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
