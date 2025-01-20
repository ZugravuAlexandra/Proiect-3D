#version 330 core

in vec3 fragPos;
in vec3 fragColor;
in vec2 TexCoord;
in vec3 normal;

out vec4 FragColor;

uniform int codCol;
uniform sampler2D myTexture;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;



void main(void)
{
    float ambientStrength = 0.6;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.8;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 finalLighting = ambient + diffuse + specular;

    vec3 texColor = texture(myTexture, TexCoord).rgb;

    FragColor = vec4(finalLighting * texColor, 1.0);
}
