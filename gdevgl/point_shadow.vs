/******************************************************************************
 * This vertex shader renders scene geometry from one face of the point light
 * shadow cubemap.
 *****************************************************************************/

#version 330 core

layout (location = 0) in vec3 vertexPosition;

uniform mat4 lightTransform;
uniform mat4 modelTransform;

out vec3 worldSpacePos;

void main()
{
    worldSpacePos = (modelTransform * vec4(vertexPosition, 1.0f)).xyz;
    gl_Position = lightTransform * vec4(worldSpacePos, 1.0f);
}