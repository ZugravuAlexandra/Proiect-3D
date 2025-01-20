#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "loadShaders.h"
// GLM includes
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
// SOIL
#include <objloader.hpp>

#include "SOIL/SOIL.h"

#include "Camera.h"
#include "Particle.h"

glm::vec3 penguinPos(0.0f, 2.0f, -80.0f);
glm::vec3 penguinVel(0.0f, 0.0f, 0.0f);
float moveSpeed = 5.0f;

glm::vec3 santaPos(0.0f, 3.0f, -3.0f);

GLuint ProgramIdModel;
GLuint VaoIdModel, VboIdModel;
GLuint VaoIdPenguin, VboIdPenguin;
GLint modelLoc, viewLocModel, projLocModel;

std::vector<glm::vec3> modelVertices, modelNormals;
std::vector<glm::vec2> modelUVs;
int nrModelVertices = 0;

std::vector<glm::vec3> modelPenguinVertices, modelPenguinNormals;
std::vector<glm::vec2> modelPenguinUVs;
int nrPenguinVertices = 0;

GLuint ProgramId, viewLocation, projLocation, modelLocation, codColLocation;
GLuint texture, textureFloor, textureWall, cubemapTexture;

GLuint VaoIdFloor, VboIdFloor, EboIdFloor;
GLuint VaoIdWall, VboIdWall, EboIdWall;
GLuint VaoIdSkybox, VboIdSkybox, EboIdSkybox;

GLuint ProgramIdSkybox;
GLint skyboxViewLoc, skyboxProjLoc;

void UpdatePenguin(float dt)
{
    glm::vec3 dir = santaPos - penguinPos;
    dir.y = 0.0f;
    float dist = glm::length(dir);
    if (dist > 0.5f) {
        dir = glm::normalize(dir);
        penguinPos += dir * moveSpeed * dt;
    }
}

void Load3DModel(const char* path)
{
    bool success = loadOBJ(path, modelVertices, modelUVs, modelNormals);
    if (!success) {
        std::cerr << "Failed to load OBJ: " << path << std::endl;
        return;
    }
    nrModelVertices = (int)modelVertices.size();
    std::cout << "Loaded model: " << path
              << " with " << nrModelVertices << " vertices." << std::endl;
}

void LoadPenguinModel(const char* path)
{
    bool success = loadOBJ(path, modelPenguinVertices, modelPenguinUVs, modelPenguinNormals);
    if (!success) {
        std::cerr << "Failed to load OBJ: " << path << std::endl;
        return;
    }
    nrPenguinVertices = (int)modelPenguinVertices.size();
    std::cout << "Loaded model: " << path
              << " with " << nrPenguinVertices << " vertices." << std::endl;
}

void CreateModelVBO()
{
    glGenVertexArrays(1, &VaoIdModel);
    glBindVertexArray(VaoIdModel);

    glGenBuffers(1, &VboIdModel);
    glBindBuffer(GL_ARRAY_BUFFER, VboIdModel);

    GLsizeiptr vertexSize = modelVertices.size() * sizeof(glm::vec3);
    GLsizeiptr normalSize = modelNormals.size() * sizeof(glm::vec3);

    glBufferData(GL_ARRAY_BUFFER, vertexSize + normalSize, NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexSize, &modelVertices[0]);
    glBufferSubData(GL_ARRAY_BUFFER, vertexSize, normalSize, &modelNormals[0]);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3, GL_FLOAT,
        GL_FALSE,
        0,
        (GLvoid*)0
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3, GL_FLOAT,
        GL_FALSE,
        0,
        (GLvoid*)vertexSize
    );

    glBindVertexArray(0);
}

void CreatePenguinVBO()
{
    glGenVertexArrays(1, &VaoIdPenguin);
    glBindVertexArray(VaoIdPenguin);

    glGenBuffers(1, &VboIdPenguin);
    glBindBuffer(GL_ARRAY_BUFFER, VboIdPenguin);

    GLsizeiptr vertexSize = modelPenguinVertices.size() * sizeof(glm::vec3);
    GLsizeiptr normalSize = modelPenguinNormals.size() * sizeof(glm::vec3);

    glBufferData(GL_ARRAY_BUFFER, vertexSize + normalSize, NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexSize, &modelPenguinVertices[0]);
    glBufferSubData(GL_ARRAY_BUFFER, vertexSize, normalSize, &modelPenguinNormals[0]);

    // in_Position --> layout(location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3, GL_FLOAT,
        GL_FALSE,
        0,
        (GLvoid*)0
    );

    // in_Normal --> layout(location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3, GL_FLOAT,
        GL_FALSE,
        0,
        (GLvoid*)vertexSize
    );

    glBindVertexArray(0);
}

void CreateModelShaders()
{
    ProgramIdModel = LoadShaders("santa.vert", "santa.frag");
    glUseProgram(ProgramIdModel);

    modelLoc   = glGetUniformLocation(ProgramIdModel, "model");
    viewLocModel = glGetUniformLocation(ProgramIdModel, "view");
    projLocModel = glGetUniformLocation(ProgramIdModel, "projection");

    glUseProgram(0);
}

void CreateSkyboxShaders() {
    ProgramIdSkybox = LoadShaders("skybox.vert", "skybox.frag");
    glUseProgram(ProgramIdSkybox);

    skyboxViewLoc = glGetUniformLocation(ProgramIdSkybox, "view");
    skyboxProjLoc = glGetUniformLocation(ProgramIdSkybox, "projection");
}

GLuint ProgramIdParticles;
GLint particleViewLoc, particleProjLoc, particleModelLoc;
GLuint VaoIdParticle, VboIdParticle;

void CreateParticleShaders() {
    ProgramIdParticles = LoadShaders("particle.vert", "particle.frag");
    glUseProgram(ProgramIdParticles);

    particleViewLoc = glGetUniformLocation(ProgramIdParticles, "view");
    particleProjLoc = glGetUniformLocation(ProgramIdParticles, "projection");
    particleModelLoc = glGetUniformLocation(ProgramIdParticles, "model");

    glUseProgram(0);
}

Camera camera(glm::vec3(0.0f, 100.0f, -600.0f));

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool firstMouse = true;
float lastX = 0.0f;
float lastY = 0.0f;

GLint winWidth = 1000, winHeight = 600;

glm::mat4 view, projection;

std::string facesCubemap[6] = {
    // "right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"
    "nx.jpg", "px.jpg", "py.jpg", "ny.jpg", "nz.jpg", "pz.jpg"
};

const int MAX_PARTICLES = 1000000;
std::vector<Particle> particles;

void InitParticles() {
    particles.clear();
    particles.resize(MAX_PARTICLES);

    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].position = glm::vec3(0.0f, 50.0f, 0.0f);

        float speed = 5.0f + (rand() % 10) * 0.5f;
        particles[i].velocity = glm::vec3(
            (rand() % 100 - 50) * 0.01f,
            speed,
            (rand() % 100 - 50) * 0.01f
        );

        particles[i].life = 2.0f + (rand() % 100) * 0.01f;

        particles[i].color = glm::vec4(0.5f, 0.0f, 0.5f, 1.0f);
    }
}

void RespawnParticle(int i) {
    float radius = 15.0f;
    float r = radius * sqrt((float)rand() / RAND_MAX);
    float angle = 2.0f * 3.14159f * ((float)rand() / RAND_MAX);
    float xOffset = r * cos(angle);
    float zOffset = r * sin(angle);

    float baseHeight = 0.0f;
    particles[i].position = glm::vec3(xOffset, baseHeight, zOffset);

    float speed = 7.0f + (rand() % 10) * 1.0f;
    particles[i].velocity = glm::vec3(
        (rand() % 100 - 50) * 0.02f,
        speed,
        (rand() % 100 - 50) * 0.02f
    );

    particles[i].life = 3.0f + (rand() % 100) * 0.01f;

    particles[i].color = glm::vec4(0.5f, 0.0f, 0.5f, 1.0f);
}

void UpdateParticles(float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle &p = particles[i];
        if (p.life > 0.0f) {
            p.life -= dt;

            p.position += p.velocity * dt;

            float gravity = -6.0f;
            p.velocity.y += gravity * 0.3f * dt;

            float maxLife = 7.0f;
            float ratio = 1.0f - (p.life / maxLife);

            p.color = glm::mix(
                glm::vec4(0.5f, 0.0f, 0.5f, 1.0f),
                glm::vec4(0.5f, 0.5f, 1.0f, 0.0f),
                ratio
            );

        } else {
            RespawnParticle(i);
        }
    }
}

void LoadCubemapTexture(void) {
    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (int i = 0; i < 6; i++) {
        int width, height, nrChannels;
        std::cout << facesCubemap[i].c_str() << std::endl;
        unsigned char *image = SOIL_load_image(facesCubemap[i].c_str(),
                                               &width, &height, &nrChannels, 0);
        if (image) {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height,
                0, GL_RGB, GL_UNSIGNED_BYTE, image
            );
            std::cout << "Succesfully loaded cubemap texture: " << facesCubemap[i] << std::endl;
            SOIL_free_image_data(image);
        } else {
            std::cout << "Failed to load cubemap face: " << facesCubemap[i] << std::endl;
            SOIL_free_image_data(image);
        }
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void LoadTexture(const char *texturePath, GLuint &texture) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height;
    unsigned char *image = SOIL_load_image(texturePath, &width, &height, 0, SOIL_LOAD_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ProcessNormalKeys(unsigned char key, int x, int y) {
    switch (key) {
        case 27: // ESC
            glutLeaveMainLoop();
            break;
        case 'w':
        case 'W': // forward
            camera.ProcessKeyboard(FORWARD, deltaTime);
            break;
        case 's':
        case 'S': // backward
            camera.ProcessKeyboard(BACKWARD, deltaTime);
            break;
        case 'a':
        case 'A': // left
            camera.ProcessKeyboard(LEFT, deltaTime);
            break;
        case 'd':
        case 'D': // right
            camera.ProcessKeyboard(RIGHT, deltaTime);
            break;
    }
    glutPostRedisplay();
}

void MouseMotion(int x, int y) {
    float xpos = static_cast<float>(x);
    float ypos = static_cast<float>(y);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
    glutPostRedisplay();
}

float penguinAngle = 0.0f;
void IdleFunction(void) {
    float currentFrame = 0.001f * glutGet(GLUT_ELAPSED_TIME);
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    UpdateParticles(deltaTime);

    UpdatePenguin(deltaTime);

    penguinAngle += 50.0f * deltaTime;

    if (penguinAngle >= 360.0f) {
        penguinAngle -= 360.0f;
    }

    glutPostRedisplay();
}

void CreateShaders(void) {
    ProgramId = LoadShaders("shader.vert", "shader.frag");
    glUseProgram(ProgramId);
}

void CreateParticleBuffers() {
    glGenVertexArrays(1, &VaoIdParticle);
    glBindVertexArray(VaoIdParticle);

    glGenBuffers(1, &VboIdParticle);
    glBindBuffer(GL_ARRAY_BUFFER, VboIdParticle);

    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * (sizeof(float) * 7),
                 nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3, GL_FLOAT,
        GL_FALSE,
        sizeof(float) * 7,
        (void*)0
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        4, GL_FLOAT,
        GL_FALSE,
        sizeof(float) * 7,
        (void*)(sizeof(float) * 3)
    );

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void UpdateParticleBuffers() {
    static std::vector<float> cpuBuffer;
    cpuBuffer.resize(MAX_PARTICLES * 7);

    for (int i = 0; i < MAX_PARTICLES; i++) {
        cpuBuffer[i*7 + 0] = particles[i].position.x;
        cpuBuffer[i*7 + 1] = particles[i].position.y;
        cpuBuffer[i*7 + 2] = particles[i].position.z;

        cpuBuffer[i*7 + 3] = particles[i].color.r;
        cpuBuffer[i*7 + 4] = particles[i].color.g;
        cpuBuffer[i*7 + 5] = particles[i].color.b;
        cpuBuffer[i*7 + 6] = particles[i].color.a;
    }

    glBindBuffer(GL_ARRAY_BUFFER, VboIdParticle);

    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    sizeof(float)*7*MAX_PARTICLES,
                    cpuBuffer.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RenderParticles() {
    UpdateParticleBuffers();

    glUseProgram(ProgramIdParticles);

    glm::mat4 viewMatrix = camera.GetViewMatrix();

    float aspect = (float)winWidth / (float)winHeight;
    glm::mat4 projectionMatrix = glm::perspective(
        glm::radians(45.0f),
        aspect,
        0.1f,
        1000.0f
    );

    glUniformMatrix4fv(particleViewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(particleProjLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glBindVertexArray(VaoIdParticle);

    glm::mat4 model = glm::translate(glm::mat4(1.0f),
    glm::vec3(-35.0f, 20.0f, -200.0f));
    glUniformMatrix4fv(particleModelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_POINTS, 0, MAX_PARTICLES);

    model = glm::translate(glm::mat4(1.0f),
                glm::vec3(35.0f, 20.0f, -200.0f));
    glUniformMatrix4fv(particleModelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_POINTS, 0, MAX_PARTICLES);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
}

void CreateVAOFloor(void) {
    GLfloat VerticesFloor[] =
    {
        // Top
        -20.0f, 0.0f, -20.0f, 1.0f, // v0
        20.0f, 0.0f, -20.0f, 1.0f, // v1
        20.0f, 0.0f, 20.0f, 1.0f, // v2
        -20.0f, 0.0f, 20.0f, 1.0f, // v3

        // Bottom
        -20.0f, -6.0f, -20.0f, 1.0f, // v4
        20.0f, -6.0f, -20.0f, 1.0f, // v5
        20.0f, -6.0f, 20.0f, 1.0f, // v6
        -20.0f, -6.0f, 20.0f, 1.0f, // v7

        // Front
        -20.0f, -6.0f, -20.0f, 1.0f, // v8
        20.0f, -6.0f, -20.0f, 1.0f, // v9
        20.0f, 0.0f, -20.0f, 1.0f, // v10
        -20.0f, 0.0f, -20.0f, 1.0f, // v11

        // Back
        -20.0f, -6.0f, 20.0f, 1.0f, // v12
        20.0f, -6.0f, 20.0f, 1.0f, // v13
        20.0f, 0.0f, 20.0f, 1.0f, // v14
        -20.0f, 0.0f, 20.0f, 1.0f, // v15

        // Left
        -20.0f, -6.0f, -20.0f, 1.0f, // v16
        -20.0f, -6.0f, 20.0f, 1.0f, // v17
        -20.0f, 0.0f, 20.0f, 1.0f, // v18
        -20.0f, 0.0f, -20.0f, 1.0f, // v19

        // Right
        20.0f, -6.0f, -20.0f, 1.0f, // v20
        20.0f, -6.0f, 20.0f, 1.0f, // v21
        20.0f, 0.0f, 20.0f, 1.0f, // v22
        20.0f, 0.0f, -20.0f, 1.0f // v23
    };

    GLfloat TexCoordsFloor[] =
    {
        // Top
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,

        // Bottom
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,

        // Front
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,

        // Back
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,

        // Left
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,

        // Right
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };

    GLfloat ColorsFloor[] =
    {
        // Top
        0.8f, 0.8f, 0.8f, // v0
        0.8f, 0.8f, 0.8f, // v1
        0.8f, 0.8f, 0.8f, // v2
        0.8f, 0.8f, 0.8f, // v3

        // Bottom
        0.6f, 0.6f, 0.6f, // v4
        0.6f, 0.6f, 0.6f, // v5
        0.6f, 0.6f, 0.6f, // v6
        0.6f, 0.6f, 0.6f, // v7

        // Front
        0.7f, 0.7f, 0.7f, // v8
        0.7f, 0.7f, 0.7f, // v9
        0.7f, 0.7f, 0.7f, // v10
        0.7f, 0.7f, 0.7f, // v11

        // Back
        0.7f, 0.7f, 0.7f, // v12
        0.7f, 0.7f, 0.7f, // v13
        0.7f, 0.7f, 0.7f, // v14
        0.7f, 0.7f, 0.7f, // v15

        // Left
        0.75f, 0.75f, 0.75f, // v16
        0.75f, 0.75f, 0.75f, // v17
        0.75f, 0.75f, 0.75f, // v18
        0.75f, 0.75f, 0.75f, // v19

        // Right
        0.75f, 0.75f, 0.75f, // v20
        0.75f, 0.75f, 0.75f, // v21
        0.75f, 0.75f, 0.75f, // v22
        0.75f, 0.75f, 0.75f // v23
    };

    GLushort IndicesFloor[] =
    {
        // Top
        0, 1, 2,
        2, 3, 0,

        // Bottom
        4, 5, 6,
        6, 7, 4,

        // Front
        8, 9, 10,
        10, 11, 8,

        // Back
        12, 13, 14,
        14, 15, 12,

        // Left
        16, 17, 18,
        18, 19, 16,

        // Right
        20, 21, 22,
        22, 23, 20
    };

    GLfloat NormalsFloor[] =
{
        // Top (0, +1, 0)
        0.0f, 1.0f, 0.0f,  // v0
        0.0f, 1.0f, 0.0f,  // v1
        0.0f, 1.0f, 0.0f,  // v2
        0.0f, 1.0f, 0.0f,  // v3

        // Bottom (0, -1, 0)
        0.0f, -1.0f, 0.0f, // v4
        0.0f, -1.0f, 0.0f, // v5
        0.0f, -1.0f, 0.0f, // v6
        0.0f, -1.0f, 0.0f, // v7

        // Front (0, 0, -1)
        0.0f, 0.0f, -1.0f, // v8
        0.0f, 0.0f, -1.0f, // v9
        0.0f, 0.0f, -1.0f, // v10
        0.0f, 0.0f, -1.0f, // v11

        // Back (0, 0, +1)
        0.0f, 0.0f, 1.0f,  // v12
        0.0f, 0.0f, 1.0f,  // v13
        0.0f, 0.0f, 1.0f,  // v14
        0.0f, 0.0f, 1.0f,  // v15

        // Left (-1, 0, 0)
        -1.0f, 0.0f, 0.0f, // v16
        -1.0f, 0.0f, 0.0f, // v17
        -1.0f, 0.0f, 0.0f, // v18
        -1.0f, 0.0f, 0.0f, // v19

        // Right (+1, 0, 0)
        1.0f, 0.0f, 0.0f,  // v20
        1.0f, 0.0f, 0.0f,  // v21
        1.0f, 0.0f, 0.0f,  // v22
        1.0f, 0.0f, 0.0f   // v23
    };

    glGenVertexArrays(1, &VaoIdFloor);
    glBindVertexArray(VaoIdFloor);

    glGenBuffers(1, &VboIdFloor);
    glGenBuffers(1, &EboIdFloor);

    glBindBuffer(GL_ARRAY_BUFFER, VboIdFloor);

    GLsizeiptr posSize = sizeof(VerticesFloor);
    GLsizeiptr colSize = sizeof(ColorsFloor);
    GLsizeiptr texSize = sizeof(TexCoordsFloor);
    GLsizeiptr norSize = sizeof(NormalsFloor);
    GLsizeiptr totalSize = posSize + colSize + texSize + norSize;

    glBufferData(GL_ARRAY_BUFFER, totalSize, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,          posSize, VerticesFloor);
    glBufferSubData(GL_ARRAY_BUFFER, posSize,    colSize, ColorsFloor);
    glBufferSubData(GL_ARRAY_BUFFER, posSize + colSize, texSize, TexCoordsFloor);
    glBufferSubData(GL_ARRAY_BUFFER, posSize + colSize + texSize, norSize, NormalsFloor);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboIdFloor);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IndicesFloor), IndicesFloor, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 4, GL_FLOAT, GL_FALSE,
        0, (GLvoid*)0
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        0, (GLvoid*)(posSize)
    );

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE,
        0, (GLvoid*)(posSize + colSize)
    );

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3, 3, GL_FLOAT, GL_FALSE,
        0, (GLvoid*)(posSize + colSize + texSize)
    );

    glBindVertexArray(0);
}

void CreateVAOWall(void) {
    GLfloat VerticesWall[] = {
        // Top
        -50.0f, 10.0f, -20.0f, 1.0f, // v0
        -20.0f, 10.0f, -20.0f, 1.0f, // v1
        -20.0f, 10.0f, 20.0f, 1.0f, // v2
        -50.0f, 10.0f, 20.0f, 1.0f, // v3

        // Bottom
        -50.0f, -6.0f, -20.0f, 1.0f, // v4
        -20.0f, -6.0f, -20.0f, 1.0f, // v5
        -20.0f, -6.0f, 20.0f, 1.0f, // v6
        -50.0f, -6.0f, 20.0f, 1.0f, // v7

        // Front
        -50.0f, -6.0f, -20.0f, 1.0f, // v8
        -20.0f, -6.0f, -20.0f, 1.0f, // v9
        -20.0f, 10.0f, -20.0f, 1.0f, // v10
        -50.0f, 10.0f, -20.0f, 1.0f, // v11

        // Back
        -50.0f, -6.0f, 20.0f, 1.0f, // v12
        -20.0f, -6.0f, 20.0f, 1.0f, // v13
        -20.0f, 10.0f, 20.0f, 1.0f, // v14
        -50.0f, 10.0f, 20.0f, 1.0f, // v15

        // Left
        -50.0f, -6.0f, -20.0f, 1.0f, // v16
        -50.0f, -6.0f, 20.0f, 1.0f, // v17
        -50.0f, 10.0f, 20.0f, 1.0f, // v18
        -50.0f, 10.0f, -20.0f, 1.0f, // v19

        // Right
        -20.0f, -6.0f, -20.0f, 1.0f, // v20
        -20.0f, -6.0f, 20.0f, 1.0f, // v21
        -20.0f, 10.0f, 20.0f, 1.0f, // v22
        -20.0f, 10.0f, -20.0f, 1.0f // v23
    };

    GLfloat TexCoordsWall[] = {
        // Top
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,

        // Bottom
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,

        // Front
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,

        // Back
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,

        // Left
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,

        // Right
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };


    GLfloat ColorsWall[] = {
        // Top
        0.8f, 0.8f, 0.8f, // v0
        0.8f, 0.8f, 0.8f, // v1
        0.8f, 0.8f, 0.8f, // v2
        0.8f, 0.8f, 0.8f, // v3

        // Bottom
        0.6f, 0.6f, 0.6f, // v4
        0.6f, 0.6f, 0.6f, // v5
        0.6f, 0.6f, 0.6f, // v6
        0.6f, 0.6f, 0.6f, // v7

        // Front
        0.7f, 0.7f, 0.7f, // v8
        0.7f, 0.7f, 0.7f, // v9
        0.7f, 0.7f, 0.7f, // v10
        0.7f, 0.7f, 0.7f, // v11

        // Back
        0.7f, 0.7f, 0.7f, // v12
        0.7f, 0.7f, 0.7f, // v13
        0.7f, 0.7f, 0.7f, // v14
        0.7f, 0.7f, 0.7f, // v15

        // Left
        0.75f, 0.75f, 0.75f, // v16
        0.75f, 0.75f, 0.75f, // v17
        0.75f, 0.75f, 0.75f, // v18
        0.75f, 0.75f, 0.75f, // v19

        // Right
        0.75f, 0.75f, 0.75f, // v20
        0.75f, 0.75f, 0.75f, // v21
        0.75f, 0.75f, 0.75f, // v22
        0.75f, 0.75f, 0.75f // v23
    };

    GLushort IndicesWall[] = {
        // Top
        0, 1, 2,
        2, 3, 0,

        // Bottom
        4, 5, 6,
        6, 7, 4,

        // Front
        8, 9, 10,
        10, 11, 8,

        // Back
        12, 13, 14,
        14, 15, 12,

        // Left
        16, 17, 18,
        18, 19, 16,

        // Right
        20, 21, 22,
        22, 23, 20
    };

    GLfloat NormalsWall[] =
{
        // Top (0, +1, 0)
        0.0f, 1.0f, 0.0f,  // v0
        0.0f, 1.0f, 0.0f,  // v1
        0.0f, 1.0f, 0.0f,  // v2
        0.0f, 1.0f, 0.0f,  // v3

        // Bottom (0, -1, 0)
        0.0f, -1.0f, 0.0f, // v4
        0.0f, -1.0f, 0.0f, // v5
        0.0f, -1.0f, 0.0f, // v6
        0.0f, -1.0f, 0.0f, // v7

        // Front (0, 0, -1)
        0.0f, 0.0f, -1.0f, // v8
        0.0f, 0.0f, -1.0f, // v9
        0.0f, 0.0f, -1.0f, // v10
        0.0f, 0.0f, -1.0f, // v11

        // Back (0, 0, +1)
        0.0f, 0.0f, 1.0f,  // v12
        0.0f, 0.0f, 1.0f,  // v13
        0.0f, 0.0f, 1.0f,  // v14
        0.0f, 0.0f, 1.0f,  // v15

        // Left (-1, 0, 0)
        -1.0f, 0.0f, 0.0f, // v16
        -1.0f, 0.0f, 0.0f, // v17
        -1.0f, 0.0f, 0.0f, // v18
        -1.0f, 0.0f, 0.0f, // v19

        // Right (+1, 0, 0)
        1.0f, 0.0f, 0.0f,  // v20
        1.0f, 0.0f, 0.0f,  // v21
        1.0f, 0.0f, 0.0f,  // v22
        1.0f, 0.0f, 0.0f   // v23
    };

    glGenVertexArrays(1, &VaoIdWall);
    glBindVertexArray(VaoIdWall);

    glGenBuffers(1, &VboIdWall);
    glGenBuffers(1, &EboIdWall);

    glBindBuffer(GL_ARRAY_BUFFER, VboIdWall);
    GLsizeiptr posSize = sizeof(VerticesWall);
    GLsizeiptr colSize = sizeof(ColorsWall);
    GLsizeiptr texSize = sizeof(TexCoordsWall);
    GLsizeiptr normSize = sizeof(NormalsWall);
    GLsizeiptr totalSize = posSize + colSize + texSize + normSize;

    glBufferData(GL_ARRAY_BUFFER, totalSize, NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, posSize, VerticesWall);
    glBufferSubData(GL_ARRAY_BUFFER, posSize, colSize, ColorsWall);
    glBufferSubData(GL_ARRAY_BUFFER, posSize + colSize, texSize, TexCoordsWall);
    glBufferSubData(GL_ARRAY_BUFFER, posSize + colSize + texSize, normSize, NormalsWall);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboIdWall);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IndicesWall), IndicesWall, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (posSize));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (posSize + colSize));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (posSize + colSize + texSize));

    glBindVertexArray(0);
}

void CreateVAOSkybox(void) {
    GLfloat SkyboxVertices[] =
    {
        -1.0f, -1.0f, 1.0f, // 0
        1.0f, -1.0f, 1.0f, // 1
        1.0f, -1.0f, -1.0f, // 2
        -1.0f, -1.0f, -1.0f, // 3
        -1.0f, 1.0f, 1.0f, // 4
        1.0f, 1.0f, 1.0f, // 5
        1.0f, 1.0f, -1.0f, // 6
        -1.0f, 1.0f, -1.0f // 7
    };

    GLushort SkyboxIndices[] = {
        // Front
        0, 1, 5,
        5, 4, 0,

        // Right
        1, 2, 6,
        6, 5, 1,

        // Back
        2, 3, 7,
        7, 6, 2,

        // Left
        3, 0, 4,
        4, 7, 3,

        // Top
        4, 5, 6,
        6, 7, 4,

        // Bottom
        3, 2, 1,
        1, 0, 3
    };

    glGenVertexArrays(1, &VaoIdSkybox);
    glGenBuffers(1, &VboIdSkybox);
    glGenBuffers(1, &EboIdSkybox);

    glBindVertexArray(VaoIdSkybox);

    glBindBuffer(GL_ARRAY_BUFFER, VboIdSkybox);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SkyboxVertices), SkyboxVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboIdSkybox);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(SkyboxIndices), SkyboxIndices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3, GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        (GLvoid *) 0
    );

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void DestroyShaders(void) {
    glDeleteProgram(ProgramId);
}

void DestroyModel()
{
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &VboIdModel);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VaoIdModel);

    glDeleteProgram(ProgramIdModel);
}

void DestroyVBO(void) {
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    // Floor
    glDeleteBuffers(1, &VboIdFloor);
    glDeleteBuffers(1, &EboIdFloor);
    glDeleteVertexArrays(1, &VaoIdFloor);

    // Wall
    glDeleteBuffers(1, &VboIdWall);
    glDeleteBuffers(1, &EboIdWall);
    glDeleteVertexArrays(1, &VaoIdWall);

    // Skybox
    glDeleteBuffers(1, &VboIdSkybox);
    glDeleteBuffers(1, &EboIdSkybox);
    glDeleteVertexArrays(1, &VaoIdSkybox);

    // Particles
    glDeleteBuffers(1, &VboIdParticle);
    glDeleteVertexArrays(1, &VaoIdParticle);
}

void Cleanup(void) {
    DestroyShaders();
    DestroyVBO();
    DestroyModel();
}

void Initialize(void) {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_PROGRAM_POINT_SIZE);

    LoadTexture("texture_floor.jpg", textureFloor);
    LoadTexture("ice_wall.jpg", textureWall);
    LoadCubemapTexture();

    CreateVAOFloor();
    CreateVAOWall();
    CreateVAOSkybox();

    CreateShaders();
    CreateSkyboxShaders();

    InitParticles();
    CreateParticleBuffers();
    CreateParticleShaders();

    Load3DModel("mosu1.obj");

    CreateModelVBO();
    CreateModelShaders();

    LoadPenguinModel("pinguin2.obj");
    CreatePenguinVBO();

    modelLocation = glGetUniformLocation(ProgramId, "modelShader");
    viewLocation  = glGetUniformLocation(ProgramId, "viewShader");
    projLocation  = glGetUniformLocation(ProgramId, "projectionShader");
    codColLocation = glGetUniformLocation(ProgramId, "codCol");
}

void SetMVP(void) {
    glm::mat4 view = camera.GetViewMatrix();

    float aspect = (float) winWidth / (float) winHeight;
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        aspect,
        0.1f,
        1000.0f
    );

    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(projection));
}

void RenderFunction(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(ProgramId);

    GLint lightPosLoc    = glGetUniformLocation(ProgramId, "lightPos");
    GLint viewPosLoc     = glGetUniformLocation(ProgramId, "viewPos");
    GLint lightColorLoc  = glGetUniformLocation(ProgramId, "lightColor");

    glUniform3f(lightPosLoc, 0.0f, 20.0f, -150.0f);

    glm::vec3 camPos = camera.Position; // or .GetPosition()
    glUniform3f(viewPosLoc, camPos.x, camPos.y, camPos.z);

    glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);

    SetMVP();

    // Floors
    glBindVertexArray(VaoIdFloor);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureFloor);
    GLint texLoc = glGetUniformLocation(ProgramId, "myTexture");
    glUniform1i(texLoc, 0);

    for (int i = 0; i < 11; i++) {
        if (i % 4 == 3) continue;

        glm::mat4 model = glm::translate(glm::mat4(1.0f),
                                         glm::vec3(0.0f, 0.0f, -i * 40.0f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
    }
    glBindVertexArray(0);

    // Walls
    glBindVertexArray(VaoIdWall);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureWall);

    for (int i = 0; i < 11; i++) {
        if (i % 4 == 3) continue;

        glm::mat4 model = glm::translate(glm::mat4(1.0f),
                                         glm::vec3(0.0f, 0.0f, -i * 40.0f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);

        model = glm::translate(glm::mat4(1.0f),
                               glm::vec3(70.0f, 0.0f, -i * 40.0f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
    }
    glBindVertexArray(0);

    // ProgramIdSkybox
    glDepthFunc(GL_LEQUAL);
    glUseProgram(ProgramIdSkybox);

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));

    float aspect = (float) winWidth / (float) winHeight;
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        aspect,
        0.1f,
        1000.0f
    );

    glUniformMatrix4fv(skyboxViewLoc, 1, GL_FALSE, glm::value_ptr(viewNoTranslation));
    glUniformMatrix4fv(skyboxProjLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(VaoIdSkybox);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    GLint skyboxLoc = glGetUniformLocation(ProgramIdSkybox, "skybox");
    glUniform1i(skyboxLoc, 0);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);
    glDepthFunc(GL_LESS);

    // Particles
    RenderParticles();

    // Model
    glUseProgram(ProgramIdModel);

    GLint lightPosLocM   = glGetUniformLocation(ProgramIdModel, "lightPos");
    GLint viewPosLocM    = glGetUniformLocation(ProgramIdModel, "viewPos");
    GLint lightColorLocM = glGetUniformLocation(ProgramIdModel, "lightColor");

    glUniform3f(lightPosLocM, 0.0f, 20.0f, 0.0f);
    glUniform3f(viewPosLocM, camPos.x, camPos.y, camPos.z);
    glUniform3f(lightColorLocM, 1.0f, 1.0f, 1.0f);

    glm::mat4 viewMatrix = camera.GetViewMatrix();
    aspect = (float) winWidth / (float) winHeight;
    glm::mat4 projectionMatrix = glm::perspective(
        glm::radians(45.0f), aspect, 0.1f, 1000.0f
    );

    glUniformMatrix4fv(viewLocModel, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(projLocModel, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(5.0f, 5.0f, 5.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 3.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    glBindVertexArray(VaoIdModel);
    glDrawArrays(GL_TRIANGLES, 0, nrModelVertices);
    glBindVertexArray(0);

    glm::mat4 penguinMatrix = glm::mat4(1.0f);

    penguinMatrix = glm::scale(penguinMatrix, glm::vec3(5.0f, 5.0f, 5.0f));
    penguinMatrix = glm::translate(penguinMatrix, penguinPos);

    float angle = glm::radians(-90.0f);
    penguinMatrix = glm::rotate(penguinMatrix, angle, glm::vec3(0.0f, 1.0f, 0.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(penguinMatrix));

    glBindVertexArray(VaoIdPenguin);
    glDrawArrays(GL_TRIANGLES, 0, nrPenguinVertices);

    penguinMatrix = glm::mat4(1.0f);
    penguinMatrix = glm::scale(penguinMatrix, glm::vec3(15.0f, 15.0f, 15.0f));
    penguinMatrix = glm::translate(penguinMatrix, glm::vec3(10.0f, 0.0f, 0.0f));
    angle = glm::radians(180.0f);
    penguinMatrix = glm::rotate(
    penguinMatrix,
    glm::radians(penguinAngle),
    glm::vec3(0.0f, 1.0f, 0.0f)
);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(penguinMatrix));
    glDrawArrays(GL_TRIANGLES, 0, nrPenguinVertices);

    penguinMatrix = glm::mat4(1.0f);
    penguinMatrix = glm::scale(penguinMatrix, glm::vec3(15.0f, 15.0f, 15.0f));
    penguinMatrix = glm::translate(penguinMatrix, glm::vec3(-10.0f, 0.0f, 0.0f));
    angle = glm::radians(180.0f);
    penguinMatrix = glm::rotate(
    penguinMatrix,
    glm::radians(penguinAngle),
    glm::vec3(0.0f, 1.0f, 0.0f)
);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(penguinMatrix));
    glDrawArrays(GL_TRIANGLES, 0, nrPenguinVertices);


    glBindVertexArray(0);

    glutSwapBuffers();
    glFlush();
}

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(winWidth, winHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Icy Run");

    glewInit();

    Initialize();

    glutDisplayFunc(RenderFunction);
    glutIdleFunc(IdleFunction);
    glutKeyboardFunc(ProcessNormalKeys);
    glutPassiveMotionFunc(MouseMotion);

    glutCloseFunc(Cleanup);
    glutMainLoop();

    return 0;
}