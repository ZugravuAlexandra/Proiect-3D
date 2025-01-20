#ifndef PARTICLE_H
#define PARTICLE_H
#include <GL/glew.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct Particle {
  glm::vec3 position;
  glm::vec3 velocity;
  GLfloat life;
  glm::vec4 color;
};

#endif //PARTICLE_H
