/******************************************************************************
 * This fragment shader is exactly the same as in the previous demo. ;)
 *
 * Happy hacking! - eric
 *****************************************************************************/

#version 330 core

in vec3 worldSpacePos;
in vec3 worldSpaceNorm;
in vec3 objColor;
in vec3 worldTangent;
in vec3 worldBitangent;

in vec3 shaderColor;
in vec2 shaderTexCoord;
uniform sampler2D shaderTexture;
uniform sampler2D specularTexture; // specular texture for the coin
uniform sampler2D normalMap;       // normal map for normal-mapped objects
uniform int useSpecularTexture;    // 1 = use, 0 = don't use
uniform int useNormalMap;          // 1 = use normal mapping, 0 = vertex normals only
// Make highlights sharper so the specular toggle is more obvious
float shininess = 64.0f;
out vec4 fragmentColor;

uniform vec3 cameraPos; 
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float specColor;

// Slightly reduce ambient so specular pops more
float ambColor = 0.2;
float constant = 1.0f;
float linear = 0.22f;
float quadratic = 0.2f;

uniform vec3 spotPosition;
uniform vec3 spotDirection;
uniform float spotCutoff;   // in radians
uniform vec3 spotColor;


void main()
{
    vec3 texColor = texture(shaderTexture, shaderTexCoord).rgb;
    vec3 norm = normalize(worldSpaceNorm);

    // Optional normal mapping: perturb the normal using the normal map in tangent space
    if (useNormalMap == 1) {
        vec3 T = normalize(worldTangent);
        vec3 B = normalize(worldBitangent);
        vec3 N = normalize(worldSpaceNorm);
        mat3 TBN = mat3(T, B, N);
        vec3 mapN = texture(normalMap, shaderTexCoord).rgb * 2.0 - 1.0;
        norm = normalize(TBN * mapN);
    }
    vec3 viewDir = normalize(cameraPos - worldSpacePos);
    
    // Sample specular map once
    vec3 specularSample = texture(specularTexture, shaderTexCoord).rgb;
    float specMapIntensity = max(specularSample.r, max(specularSample.g, specularSample.b));
    
    // ==================== POINT LIGHT ====================
    float lightWorldDistance = length(lightPosition - worldSpacePos);
    float attenuation = 1.0 / (constant + linear * lightWorldDistance + quadratic * (lightWorldDistance * lightWorldDistance));
    vec3 lightVec = normalize(lightPosition - worldSpacePos);
    
    // Diffuse (Lambertian)
    float diffuse = max(dot(norm, lightVec), 0.0);
    
    // Specular (Blinn-Phong: half-vector for wider highlights)
    vec3 halfVec = normalize(lightVec + viewDir);
    float spec = pow(max(dot(norm, halfVec), 0.0), shininess);
    
    // Point light contributions
    vec3 pointDiffuse = lightColor * diffuse * attenuation;
    vec3 pointAmbient = lightColor * ambColor * attenuation;
    
    // Point light specular - modulated by specular map when enabled
    vec3 pointSpecular = vec3(0.0);
    if (useSpecularTexture == 1) {
        // Stronger specular so the ON/OFF toggle is clearly visible
        pointSpecular = lightColor * specColor * 3.0 * spec * specMapIntensity * attenuation;
    }
    
    // ==================== SPOTLIGHT ====================
    vec3 spotLightVec = normalize(spotPosition - worldSpacePos);
    
    // Spotlight cone calculation
    float theta = dot(normalize(-spotLightVec), normalize(spotDirection));
    float outerCutoff = spotCutoff - 0.05; // small fade zone
    float epsilon = spotCutoff - outerCutoff;
    float spotIntensity = clamp((theta - outerCutoff) / epsilon, 0.0, 1.0);
    
    // Diffuse (Lambertian)
    float spotDiff = max(dot(norm, spotLightVec), 0.0);
    
    // Specular (Blinn-Phong)
    vec3 spotHalfVec = normalize(spotLightVec + viewDir);
    float spotSpec = pow(max(dot(norm, spotHalfVec), 0.0), shininess);
    
    // Spotlight contributions (affected by cone intensity only - no distance attenuation for spotlight)
    vec3 spotDiffuse = spotColor * spotDiff * spotIntensity;
    vec3 spotAmbient = spotColor * ambColor * spotIntensity * 0.3; // Less ambient from spot
    
    // Spotlight specular - modulated by specular map when enabled
    vec3 spotSpecular = vec3(0.0);
    if (useSpecularTexture == 1) {
        // Match point light: amplify specular contribution when enabled
        spotSpecular = spotColor * specColor * 3.0 * spotSpec * specMapIntensity * spotIntensity;
    }
    
    // ==================== COMBINE ====================
    // Diffuse and ambient are multiplied by texture color
    vec3 finalLitColor = texColor * (pointDiffuse + pointAmbient + spotDiffuse + spotAmbient);
    
    // Specular highlights are added on top (not multiplied by texture - they're reflections of light)
    finalLitColor += pointSpecular + spotSpecular;

    fragmentColor = vec4(finalLitColor, 1.0f);
}
