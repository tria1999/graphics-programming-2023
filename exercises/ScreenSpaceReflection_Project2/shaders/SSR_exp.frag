#version 330 core

in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D colorMap;
uniform vec3 eyePos;
uniform mat4 invViewProj;

const int MAX_STEPS = 64;
const float MAX_DIST = 100.0;
const float STEP_SIZE = 0.1;

vec3 trace(vec3 origin, vec3 direction, int steps)
{
    float dist = 0.0;
    for (int i = 0; i < MAX_STEPS; i++)
    {
        vec3 pos = origin + direction * dist;
        dist += STEP_SIZE;
        if (dist > MAX_DIST)
            break;
        if (texture(colorMap, fragTexCoord).a > 0.0)
            return pos;
    }
    return vec3(0.0);
}

void main()
{
    vec3 ray = normalize((invViewProj * vec4(fragTexCoord * 2.0 - 1.0, 1.0, 1.0)).xyz);
    vec3 hit = trace(eyePos, ray, MAX_STEPS);
    if (hit == vec3(0.0))
        discard;
    vec3 normal = normalize(hit - eyePos);
    vec3 reflected = reflect(ray, normal);
    vec2 reflectedTexCoord = (reflected.xy / reflected.z) * 0.5 + 0.5;
    vec3 reflectedHit = trace(hit + reflected * STEP_SIZE, reflected, MAX_STEPS);
    if (reflectedHit == vec3(0.0))
        discard;
    fragColor = mix(texture(colorMap, fragTexCoord), texture(colorMap, reflectedTexCoord), 0.5);
}
