#version 330 core

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec3 in_Color;
layout(location = 2) in vec2 in_TexCoord;
layout(location = 3) in vec3 in_Normal;

out vec3 fragPos;
out vec3 fragColor;
out vec2 TexCoord;
out vec3 normal;

uniform mat4 viewShader;
uniform mat4 projectionShader;
uniform mat4 modelShader;

void main(void)
{

    vec4 worldPos = modelShader * in_Position;
    gl_Position = projectionShader * viewShader * worldPos;


    fragPos = worldPos.xyz;


    fragColor = in_Color;
    TexCoord = in_TexCoord;


    mat3 normalMatrix = mat3(transpose(inverse(modelShader)));
    normal = normalize(normalMatrix * in_Normal);
}
