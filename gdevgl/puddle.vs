#version 330 core

// Puddle quad inputs
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 vertexTexCoord;

// Transform matrices
uniform mat4 modelTransform;
uniform mat4 viewTransform;
uniform mat4 projectionTransform;
uniform mat4 reflectionViewProjection;

// Outputs for the fragment shader
out vec3 worldSpacePos;
out vec2 puddleTexCoord;
out vec4 reflectionClipPos;

// The transform matrices are set up in such a way that the reflectionViewProjection will mirror the world space across the puddle plane
// I bascially ctrl c then (ctrl v * reflection matrix) to make this happen
void main()
{
    vec4 worldPosition = modelTransform * vec4(vertexPosition, 1.0f);
    worldSpacePos = worldPosition.xyz;
    puddleTexCoord = vertexTexCoord;
    reflectionClipPos = reflectionViewProjection * worldPosition; // Here's where I ctrl v * reflection matrix the thing
    gl_Position = projectionTransform * viewTransform * worldPosition;
} // P.S. I'm so sorry my comments are getting less professional and more conversational 
// - it is quite literally helping to keep me sane at this point