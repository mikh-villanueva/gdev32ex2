/******************************************************************************
 * This fragment shader stores normalized radial depth for point-light shadow
 * mapping.
 *****************************************************************************/

#version 330 core

in vec3 worldSpacePos;

uniform vec3 lightPosition;
uniform float farPlane;

void main()
{
    float lightDistance = length(worldSpacePos - lightPosition);
    gl_FragDepth = lightDistance / farPlane;
}