#version 330 core

in vec3 fragNormal;
out vec4 outColor;

void main()
{
    vec3 color = normalize(fragNormal) * 0.5 + 0.5;
    outColor = vec4(color, 1.0);
}
