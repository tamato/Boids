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
        Positions[i] = glm::vec3(0,0,-10);
        Velocities[i] = glm::ballRand(1.0f);    // random within a volume
        Radii[i] = 0;
    }

    Constraint.Count = 100;
    Constraint.Radii.resize(Constraint.Count);
    Constraint.Points.resize(Constraint.Count);
    for (uint32_t i = 0; i < Constraint.Count; ++i){
        Constraint.Radii[i] = 5.0f;

        float x = std::cos(i/float(Constraint.Count)) * Constraint.Radii[i];
        float y = std::sin(i/float(Constraint.Count)) * Constraint.Radii[i];
        Constraint.Points[i] = glm::vec3(x,0,y);
    }
}

void Particles::update(float dt)
{

    /*
    for each particle find current flockmates
    calculate the groups average position (Cohesion)
    calculate the groups average direction (Alingment)
    
    udpate position on velocity
     

    */

    for (int i=0; i<Count; ++i){
        float speed = glm::length(Velocities[i]);
        Velocities[i] += glm::sphericalRand(speed);     // random along the surface of a sphere
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
