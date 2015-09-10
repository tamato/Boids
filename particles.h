#ifndef PARTICLE_H_
#define PARTICLE_H_

#include <vector>

#include "glm/glm.hpp"

struct Particles
{
    int Count;
    float Alingment;
    float Cohesion;
    float Seperation;

    typedef glm::vec3 position_type;
    std::vector<glm::vec3> Velocities;
    std::vector<glm::vec3> Positions;
    std::vector<float> Radii;

    // collision radii
    // visual radii
    // search radii

    // values with curr/max
    // life
    // 

    Particles();
    void init(int count);
    void update(float dt);
    void shutdown();
};

#endif //PARTICLE_H_