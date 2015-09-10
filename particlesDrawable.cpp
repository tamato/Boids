#include "particlesDrawable.h"

#define GLEW_NO_GLU
#include <GL/glew.h>

namespace {
    #define bufferOffest(x) ((char*)NULL+(x))
}

void ParticlesDrawable::init()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    
    buffer_size = 0;
    point_count = 0;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, bufferOffest(0));
    glBufferData(GL_ARRAY_BUFFER, buffer_size, (GLvoid*)0, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ParticlesDrawable::update(const Particles& particles)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, buffer_size, (GLvoid*)0, GL_DYNAMIC_DRAW);
    buffer_size = particles.Count * sizeof(Particles::position_type);
    point_count = particles.Count;

    glBufferData(GL_ARRAY_BUFFER, buffer_size, (GLvoid*)particles.Positions.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticlesDrawable::render()
{
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, point_count);
    glBindVertexArray(0);
}

void ParticlesDrawable::shutdown()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    vao = 0;
    vbo = 0;
    buffer_size = 0;
    point_count = 0;
}
