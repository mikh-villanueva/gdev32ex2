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
out vec4 fragmentColor;

uniform vec3 cameraPos; 
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float specColor;

float ambColor = 0.7;
float constant = 1.0f;
float linear = 0.22f;
float quadratic = 0.2f;

uniform vec3 spotPosition;
uniform vec3 spotDirection;
uniform float spotCutoff;
uniform float spotOuterCutoff;
uniform vec3 spotColor;

in vec4 shaderLightSpacePosition;
uniform sampler2D shadowMap;
uniform bool shadowsEnabled;
uniform int shadowSamplesPerAxis;
uniform float shadowFilterRadius;

float randomValue(vec2 seed)
{
    return fract(sin(dot(seed, vec2(12.9898f, 78.233f))) * 43758.5453f);
}

float getShadowAmount(vec3 norm)
{
    if (!shadowsEnabled || shaderLightSpacePosition.w <= 0.0f)
        return 0.0f;

    vec3 position = shaderLightSpacePosition.xyz / shaderLightSpacePosition.w;
    position = position * 0.5f + 0.5f;

    if (position.x < 0.0f || position.x > 1.0f
        || position.y < 0.0f || position.y > 1.0f
        || position.z < 0.0f || position.z > 1.0f)
    {
        return 0.0f;
    }

    vec2 texelSize = 1.0f / vec2(textureSize(shadowMap, 0));
    vec3 shadowLightDir = normalize(spotPosition - worldSpacePos);
    float bias = max(0.00075f, 0.003f * (1.0f - max(dot(norm, shadowLightDir), 0.0f)));

    int halfWindow = shadowSamplesPerAxis / 2;
    float occlusion = 0.0f;
    float samplesTaken = 0.0f;

    float rotationAngle = randomValue(position.xy + worldSpacePos.xz) * 6.28318530718f;
    mat2 rotation = mat2(cos(rotationAngle), -sin(rotationAngle),
                         sin(rotationAngle),  cos(rotationAngle));

    for (int x = -4; x <= 4; x++)
    {
        for (int y = -4; y <= 4; y++)
        {
            if (abs(x) > halfWindow || abs(y) > halfWindow)
                continue;

            vec2 gridOffset = vec2(float(x), float(y));
            vec2 jitter = vec2(
                randomValue(position.xy + gridOffset + worldSpacePos.xy) - 0.5f,
                randomValue(position.yx + gridOffset + worldSpacePos.zy) - 0.5f);

            vec2 sampleUv = position.xy + rotation * (gridOffset + jitter) * texelSize * shadowFilterRadius;
            float sampleDepth = texture(shadowMap, sampleUv).r;

            occlusion += (position.z - bias > sampleDepth) ? 1.0f : 0.0f;
            samplesTaken += 1.0f;
        }
    }

    return samplesTaken > 0.0f ? (occlusion / samplesTaken) : 0.0f;
}

void main()
{
    vec4 texSample = texture(shaderTexture, shaderTexCoord);
    vec3 texColor = texSample.rgb;
    vec3 norm = normalize(worldSpaceNorm);
    vec3 viewDir = normalize(cameraPos - worldSpacePos);

    float lightWorldDistance = length(lightPosition - worldSpacePos);
    float attenuation = 1.0 / (constant + linear * lightWorldDistance + quadratic * (lightWorldDistance * lightWorldDistance));
    
    vec3 lightVec = normalize(lightPosition - worldSpacePos);
    vec3 reflectVec = reflect(-lightVec, norm);
    float pointSpec = pow(max(dot(viewDir, reflectVec), 0.0), 32);

    float pointDiff = max(dot(lightVec, norm), 0.0);

    vec3 pointAmbient = ambColor * lightColor;
    vec3 pointDiffuse = pointDiff * lightColor;
    vec3 pointSpecular = specColor * pointSpec * lightColor;
    vec3 pointResult = (pointAmbient + pointDiffuse + pointSpecular) * attenuation;

    vec3 spotLightVec = normalize(spotPosition - worldSpacePos);
    float shadowAmount = getShadowAmount(norm);

    float spotDiff = max(dot(norm, spotLightVec), 0.0);

    vec3 spotReflect = reflect(-spotLightVec, norm);
    float spotSpec = pow(max(dot(viewDir, spotReflect), 0.0), 32);
    

    float theta = dot(normalize(-spotLightVec), normalize(spotDirection));

    float epsilon = max(spotCutoff - spotOuterCutoff, 0.0001f);
    float spotIntensity = clamp((theta - spotOuterCutoff) / epsilon, 0.0, 1.0);

    vec3 spotAmbient = ambColor * spotColor;
    vec3 spotDiffuse = spotDiff * spotColor;
    vec3 spotSpecular = specColor * spotSpec * spotColor;
    vec3 spotResult = (spotAmbient + (1.0f - shadowAmount) * (spotDiffuse + spotSpecular)) * spotIntensity;

    vec3 finalLight = pointResult + spotResult;
    vec3 finalLitColor = texColor * finalLight;

    fragmentColor = vec4(finalLitColor, texSample.a);
}
