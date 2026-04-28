/******************************************************************************
 * This vertex shader renders scene geometry from one face of the point light
 * shadow cubemap.
 *****************************************************************************/

#version 330 core

// Vertex input
layout (location = 0) in vec3 vertexPosition;

// Light-space and model transform matrices
uniform mat4 lightTransform;
uniform mat4 modelTransform;

// World-space position output
out vec3 worldSpacePos;

void main()
{
    // World position, then project into clip space
    worldSpacePos = (modelTransform * vec4(vertexPosition, 1.0f)).xyz;
    gl_Position = lightTransform * vec4(worldSpacePos, 1.0f);
}