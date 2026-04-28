/******************************************************************************
 * This fragment shader is exactly the same as in the previous demo. ;)
 *
 * Happy hacking! - eric
 *****************************************************************************/

#version 330 core

in vec3 worldSpacePos;
in vec3 worldSpaceNorm;
in vec3 worldTangent;
in vec3 worldBitangent;
in vec3 objColor;

in vec3 shaderColor;
in vec2 shaderTexCoord;
uniform sampler2D shaderTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalMap;
uniform int useSpecularTexture;
uniform int useNormalMap;
uniform float materialOpacity;
out vec4 fragmentColor;

uniform vec3 cameraPos;
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 secondaryLightPosition;
uniform vec3 secondaryLightColor;
uniform float specColor;

float ambColor = 0.2;
float constant = 1.0f;
float linear = 0.22f;
float quadratic = 0.2f;
float shininess = 64.0f;

uniform vec3 spotPosition;
uniform vec3 spotDirection;
uniform float spotCutoff;
uniform float spotOuterCutoff;
uniform vec3 spotColor;

in vec4 shaderLightSpacePosition;
uniform sampler2D shadowMap;
uniform samplerCube pointShadowMap;
uniform samplerCube secondaryPointShadowMap;
uniform bool shadowsEnabled;
uniform int shadowSamplesPerAxis;
uniform float shadowFilterRadius;
uniform float pointShadowFarPlane;
uniform vec3 materialEmissionColor;
uniform float materialEmissionStrength;

const vec3 pointShadowOffsets[20] = vec3[](
    vec3(0.0f, 0.0f, 0.0f),
    vec3(1.0f, 1.0f, 1.0f),
    vec3(1.0f, -1.0f, 1.0f),
    vec3(-1.0f, 1.0f, 1.0f),
    vec3(-1.0f, -1.0f, 1.0f),
    vec3(1.0f, 1.0f, -1.0f),
    vec3(1.0f, -1.0f, -1.0f),
    vec3(-1.0f, 1.0f, -1.0f),
    vec3(-1.0f, -1.0f, -1.0f),
    vec3(1.0f, 1.0f, 0.0f),
    vec3(1.0f, -1.0f, 0.0f),
    vec3(-1.0f, 1.0f, 0.0f),
    vec3(-1.0f, -1.0f, 0.0f),
    vec3(1.0f, 0.0f, 1.0f),
    vec3(1.0f, 0.0f, -1.0f),
    vec3(-1.0f, 0.0f, 1.0f),
    vec3(-1.0f, 0.0f, -1.0f),
    vec3(0.0f, 1.0f, 1.0f),
    vec3(0.0f, 1.0f, -1.0f),
    vec3(0.0f, -1.0f, 1.0f)
);

float randomValue(vec2 seed)
{
    return fract(sin(dot(seed, vec2(12.9898f, 78.233f))) * 43758.5453f);
}

vec3 getSurfaceNormal()
{
    vec3 norm = normalize(worldSpaceNorm);
    if (useNormalMap == 0)
        return norm;

    vec3 tangent = normalize(worldTangent);
    vec3 bitangent = normalize(worldBitangent);
    mat3 tbn = mat3(tangent, bitangent, norm);
    vec3 mapNormal = texture(normalMap, shaderTexCoord).rgb * 2.0f - 1.0f;
    return normalize(tbn * mapNormal);
}

float getSpecularStrength()
{
    if (useSpecularTexture == 0)
        return 0.0f;

    vec3 specularSample = texture(specularTexture, shaderTexCoord).rgb;
    return max(specularSample.r, max(specularSample.g, specularSample.b));
}

float getSpotShadowAmount(vec3 norm)
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
    float normalAlignment = max(dot(norm, shadowLightDir), 0.0f);
    float bias = max(0.00012f, 0.0012f * (1.0f - normalAlignment));

    int halfWindow = shadowSamplesPerAxis / 2;
    float occlusion = 0.0f;
    float count = 0.0f;

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
            count += 1.0f;
        }
    }

    return count > 0.0f ? (occlusion / count) : 0.0f;
}

float getPointShadowAmount(vec3 norm, vec3 pointLightPosition, samplerCube shadowSampler)
{
    if (!shadowsEnabled)
        return 0.0f;

    vec3 fragmentToLight = worldSpacePos - pointLightPosition;
    float currentDepth = length(fragmentToLight);
    if (currentDepth <= 0.0001f || currentDepth >= pointShadowFarPlane)
        return 0.0f;

    vec3 pointLightVec = normalize(pointLightPosition - worldSpacePos);
    float normalAlignment = max(dot(norm, pointLightVec), 0.0f);
    float bias = max(0.04f, 0.12f * (1.0f - normalAlignment));
    int activeSamples = shadowFilterRadius > 0.0f ? min(20, shadowSamplesPerAxis * 2 + 1) : 1;
    float diskRadius = shadowFilterRadius > 0.0f ? shadowFilterRadius * 0.08f : 0.0f;
    float occlusion = 0.0f;

    for (int sampleIndex = 0; sampleIndex < 20; ++sampleIndex)
    {
        if (sampleIndex >= activeSamples)
            break;

        vec3 sampleDirection = fragmentToLight + pointShadowOffsets[sampleIndex] * diskRadius;
        float closestDepth = texture(shadowSampler, sampleDirection).r * pointShadowFarPlane;
        occlusion += (currentDepth - bias > closestDepth) ? 1.0f : 0.0f;
    }

    return occlusion / float(activeSamples);
}

void main()
{
    vec4 texSample = texture(shaderTexture, shaderTexCoord);
    vec3 texColor = texSample.rgb;
    vec3 norm = getSurfaceNormal();
    vec3 viewDir = normalize(cameraPos - worldSpacePos);
    float specularStrength = getSpecularStrength();

    float lightDist = length(lightPosition - worldSpacePos);
    float attenuation = 1.0 / (constant + linear * lightDist + quadratic * (lightDist * lightDist));
    
    vec3 lightVec = normalize(lightPosition - worldSpacePos);
    float pointShadowAmount = getPointShadowAmount(norm, lightPosition, pointShadowMap);
    float pointDiff = max(dot(lightVec, norm), 0.0);
    vec3 pointHalfVec = normalize(lightVec + viewDir);
    float pointSpec = pointDiff > 0.0f ? pow(max(dot(norm, pointHalfVec), 0.0f), shininess) : 0.0f;

    vec3 pointAmbient = lightColor * ambColor * attenuation;
    vec3 pointDiffuse = lightColor * pointDiff * attenuation * (1.0f - pointShadowAmount);
    vec3 pointSpecular = lightColor * specColor * 3.0f * pointSpec * specularStrength * attenuation * (1.0f - pointShadowAmount);

    float secDist = length(secondaryLightPosition - worldSpacePos);
    float secondaryAttenuation = 1.0 / (constant + linear * secDist + quadratic * (secDist * secDist));

    vec3 secondaryLightVec = normalize(secondaryLightPosition - worldSpacePos);
    float secondaryPointShadowAmount = getPointShadowAmount(norm, secondaryLightPosition, secondaryPointShadowMap);
    float secondaryPointDiff = max(dot(secondaryLightVec, norm), 0.0);
    vec3 secondaryPointHalfVec = normalize(secondaryLightVec + viewDir);
    float secondaryPointSpec = secondaryPointDiff > 0.0f ? pow(max(dot(norm, secondaryPointHalfVec), 0.0f), shininess) : 0.0f;

    vec3 secondaryPointAmbient = secondaryLightColor * ambColor * secondaryAttenuation;
    vec3 secondaryPointDiffuse = secondaryLightColor * secondaryPointDiff * secondaryAttenuation * (1.0f - secondaryPointShadowAmount);
    vec3 secondaryPointSpecular = secondaryLightColor * specColor * 3.0f * secondaryPointSpec * specularStrength * secondaryAttenuation * (1.0f - secondaryPointShadowAmount);

    vec3 spotLightVec = normalize(spotPosition - worldSpacePos);
    float spotShadowAmount = getSpotShadowAmount(norm);

    float spotDiff = max(dot(norm, spotLightVec), 0.0);
    vec3 spotHalfVec = normalize(spotLightVec + viewDir);
    float spotSpec = spotDiff > 0.0f ? pow(max(dot(norm, spotHalfVec), 0.0f), shininess) : 0.0f;
    

    float theta = dot(normalize(-spotLightVec), normalize(spotDirection));

    float epsilon = max(spotCutoff - spotOuterCutoff, 0.0001f);
    float spotIntensity = clamp((theta - spotOuterCutoff) / epsilon, 0.0, 1.0);

    vec3 spotAmbient = spotColor * ambColor * spotIntensity * 0.3f;
    vec3 spotDiffuse = spotColor * spotDiff * (1.0f - spotShadowAmount) * spotIntensity;
    vec3 spotSpecular = spotColor * specColor * 3.0f * spotSpec * specularStrength * (1.0f - spotShadowAmount) * spotIntensity;

    vec3 emission = texColor * materialEmissionColor * materialEmissionStrength;

    vec3 finalLitColor = texColor * (pointAmbient + pointDiffuse
                                   + secondaryPointAmbient + secondaryPointDiffuse
                                   + spotAmbient + spotDiffuse)
                       + pointSpecular + secondaryPointSpecular + spotSpecular
                       + emission;

    fragmentColor = vec4(finalLitColor, texSample.a * materialOpacity);
}
