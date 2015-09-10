#ifndef PARTICLES_DRAWABLE_
#define PARTICLES_DRAWABLE_

#include "particles.h"

struct ParticlesDrawable
{
    unsigned int vbo;  // for positions
    unsigned int vao;  // for mesh
    unsigned int buffer_size;
    unsigned int point_count;

    void init();
    void update(const Particles& particles);
    void render();
    void shutdown();
};

#endif // PARTICLES_DRAWABLE_