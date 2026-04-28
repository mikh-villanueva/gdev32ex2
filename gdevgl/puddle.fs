#version 330 core

in vec3 worldSpacePos;
in vec2 puddleTexCoord;
in vec4 reflectionClipPos;

uniform sampler2D reflectionTexture;
uniform vec3 cameraPos;
uniform float time;

out vec4 fragmentColor;

void main()
{
    vec2 localUv = puddleTexCoord * 2.0f - 1.0f;
    float angle = atan(localUv.y, localUv.x);
    // Unreal Engine trick to making "random" distortions to a shape, but applied to a value mask here in the shader
    // make a randomly warped edge using trig functions
    float edgeWarp = 0.08f * sin(angle * 3.0f + 0.45f)
                   + 0.06f * sin(angle * 5.0f - 1.2f)
                   + 0.05f * cos(angle * 7.0f + 0.8f)
                   + 0.04f * sin(localUv.x * 6.5f + localUv.y * 4.0f)
                   - 0.05f * cos(localUv.y * 7.0f - localUv.x * 3.5f);

    float puddleRadius = 0.93f + edgeWarp;
    float radialDistance = length(vec2(localUv.x * 0.92f, localUv.y * 1.08f));
    // Finishing touch hehe smoothens the extreme edges of the puddle to make it look like the water thins out eventually
    float edgeAlpha = 1.0f - smoothstep(puddleRadius - 0.14f, puddleRadius + 0.02f, radialDistance);

    if (edgeAlpha <= 0.02f)
        discard;

    vec2 reflectionUv = reflectionClipPos.xy / max(reflectionClipPos.w, 0.0001f);
    reflectionUv = reflectionUv * 0.5f + 0.5f;

    // Trick I learned from Unreal Engine exploration! 
    // Use trig functions to add a distorting vector to the reflection coords being rendered to make it
    // look like the reflection is being warped by ripples on the water surface
    reflectionUv += vec2(
        sin(worldSpacePos.x * 1.6f + time * 1.4f),
        cos(worldSpacePos.z * 1.9f - time * 1.1f)) * 0.004f;

    if (reflectionUv.x < 0.0f || reflectionUv.x > 1.0f
        || reflectionUv.y < 0.0f || reflectionUv.y > 1.0f)
    {
        discard;
    }

    vec3 reflectionColor = texture(reflectionTexture, reflectionUv).rgb;
    vec3 waterTint = vec3(0.08f, 0.24f, 0.43f); // Make it blue so it looks like water!
    vec3 viewDirection = normalize(cameraPos - worldSpacePos);
    float fresnel = pow(1.0f - max(dot(viewDirection, vec3(0.0f, 1.0f, 0.0f)), 0.0f), 2.0f);
    float centerDarkening = smoothstep(1.0f, 0.18f, radialDistance);

    vec3 tintedReflection = mix(reflectionColor, waterTint, 0.35f + 0.15f * fresnel);
    vec3 finalColor = mix(tintedReflection, waterTint, 0.18f + 0.12f * puddleTexCoord.y);
    finalColor = mix(finalColor, finalColor * 0.86f + waterTint * 0.14f, centerDarkening * 0.35f);

    fragmentColor = vec4(finalColor, (0.38f + 0.36f * centerDarkening) * edgeAlpha);
}