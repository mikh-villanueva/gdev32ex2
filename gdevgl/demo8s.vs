/******************************************************************************
 * This vertex shader facilitates drawing a model from the point of view of a
 * light source, for the purpose of shadow mapping.
 *
 * Since we don't need any colors or texturing in a shadow map, only the vertex
 * positions are used by this shader.
 *
 * Happy hacking! - eric
 *****************************************************************************/

#version 330 core

layout (location = 0) in vec3 vertexPosition;
uniform mat4 lightTransform;
uniform mat4 modelTransform;

void main()
{
    gl_Position = lightTransform * modelTransform * vec4(vertexPosition, 1.0f);
}
