/******************************************************************************
 * This is a vertex shader that improves the previous demo's shader by
 * supporting transformation matrices: projection, view, and model transform.
 *
 * Happy hacking! - eric
 *****************************************************************************/

#version 330 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexColor;
layout (location = 2) in vec2 vertexTexCoord;
layout (location = 3) in vec3 vertexNormal;

uniform mat4 projectionTransform;
uniform mat4 viewTransform;
uniform mat4 modelTransform;
uniform mat4 normalTransform;

out vec3 shaderColor;
out vec2 shaderTexCoord;

out vec3 worldSpacePos;
out vec3 worldSpaceNorm;
out vec3 objColor;

void main()
{
    worldSpacePos = (modelTransform * vec4(vertexPosition, 1.0f)).xyz;
    worldSpaceNorm = (normalTransform * vec4(vertexNormal, 0.0f)).xyz;
    objColor = vertexColor;

    gl_Position = projectionTransform * viewTransform * modelTransform * vec4(vertexPosition, 1.0f);
    shaderColor = vertexColor;
    shaderTexCoord = vertexTexCoord;
}
