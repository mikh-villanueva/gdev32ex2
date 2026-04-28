/******************************************************************************
 * This fragment shader stores normalized radial depth for point-light shadow
 * mapping.
 *****************************************************************************/

#version 330 core

// World-space position from the vertex shader
in vec3 worldSpacePos;

// Point light position and far plane for normalizing depth
uniform vec3 lightPosition;
uniform float farPlane;

void main()
{
    // Store distance from light as depth
    float lightDistance = length(worldSpacePos - lightPosition);
    gl_FragDepth = lightDistance / farPlane;
}