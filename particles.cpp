#include "particles.h"

#include "glm/gtc/random.hpp"
#include "glm/glm.hpp"

Particles::Particles()
 : Count(0)
 , Alingment(0)
 , Cohesion(0)
 , Seperation(0)
{

}

void Particles::init(int count)
{
    Count = count;
    Positions.resize(count);
    Velocities.resize(count);
    Radii.resize(count);
    for (int i=0; i<count; ++i){
        Positions[i] = glm::vec3(0);
        Velocities[i] = glm::ballRand(1.0f);
        Radii[i] = 0;
    }
}

void Particles::update(float dt)
{
    for (int i=0; i<Count; ++i){
        float speed = glm::length(Velocities[i]);
        Velocities[i] += glm::sphericalRand(speed);
        Velocities[i] = glm::normalize(Velocities[i]);
        Velocities[i] *= speed;

        Positions[i] += Velocities[i] * dt;
    }
}

void Particles::shutdown()
{
    Count = 0;
    Velocities.clear();
    Positions.clear();
    Radii.clear();
}
