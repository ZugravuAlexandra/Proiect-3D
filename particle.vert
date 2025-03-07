#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;

out vec4 passColor;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(inPosition, 1.0);

    passColor = inColor;

    gl_PointSize = 12.0;
}
