/******************************************************************************
 * This fragment shader is exactly the same as in the previous demo. ;)
 *
 * Happy hacking! - eric
 *****************************************************************************/

#version 330 core

in vec3 worldSpacePos;
in vec3 worldSpaceNorm;
in vec3 objColor;

in vec3 shaderColor;
in vec2 shaderTexCoord;
uniform sampler2D shaderTexture;
uniform sampler2D specularMap;
out vec4 fragmentColor;

uniform vec3 cameraPos; 
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float specColor;

float ambColor = 0.3;
float constant = 1.0f;
float linear = 0.22f;
float quadratic = 0.2f;

uniform vec3 spotPosition;
uniform vec3 spotDirection;
uniform float spotCutoff;   // in radians
uniform vec3 spotColor;

uniform bool useSpecMap;

void main()
{
    vec3 texColor = texture(shaderTexture, shaderTexCoord).rgb;
    
    // POINT LIGHT
    float lightWorldDistance = length(lightPosition - worldSpacePos);
    float attenuation = 1.0 / (constant + linear * lightWorldDistance + quadratic * (lightWorldDistance * lightWorldDistance));
    
    vec3 lightVec = normalize(lightPosition - worldSpacePos);
    vec3 norm = normalize(worldSpaceNorm);

    vec3 viewDir = normalize(cameraPos - worldSpacePos);
    vec3 reflectVec = reflect(-lightVec, norm);
    float spec = pow(max(dot(viewDir, reflectVec), 0.0), 32);

    vec3 specMap = texture(specularMap, shaderTexCoord).rgb;

    vec3 specular;
    if (useSpecMap)
        specular = specMap * spec * lightColor;
    else
        specular = vec3(specColor * spec) * lightColor;

    float diffColor = max(dot(lightVec, norm), 0);

    vec3 ambient  = ambColor * lightColor;
    vec3 diffuse  = diffColor * lightColor;

    vec3 finalColor = (ambient + diffuse + specular) * attenuation;

    // SPOTLIGHT
    vec3 spotLightVec = normalize(spotPosition - worldSpacePos);

    float spotDiff = max(dot(norm, spotLightVec), 0.0);

    vec3 spotReflect = reflect(-spotLightVec, norm);
    float spotSpec = pow(max(dot(viewDir, spotReflect), 0.0), 32);
    vec3 spotSpecular;
    if (useSpecMap)
        spotSpecular = specMap * spotSpec * spotColor;
    else
        spotSpecular = vec3(specColor * spotSpec) * spotColor;

    float theta = dot(normalize(-spotLightVec), normalize(spotDirection));
    float spotIntensity = 0.0;
    float outerCutoff = spotCutoff - 0.05; // small fade zone
    float epsilon = spotCutoff - outerCutoff;
    spotIntensity = clamp((theta - outerCutoff) / epsilon, 0.0, 1.0);
    

    vec3 spotResult = (spotColor * (spotDiff + ambColor) + spotSpecular) * spotIntensity;
    
    // COMBINE
    vec3 finalLight = finalColor + spotResult;
    vec3 finalLitColor = texColor * finalLight;

    fragmentColor = vec4(finalLitColor, 1.0f);
}
