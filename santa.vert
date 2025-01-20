#version 330 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragNormal;

void main()
{
    gl_Position = projection * view * model * vec4(in_Position, 1.0);

    fragNormal = mat3(transpose(inverse(model))) * in_Normal;
}
