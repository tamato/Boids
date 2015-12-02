#ifndef PARTICLE_H_
#define PARTICLE_H_

#include "common/path.h"

#include <vector>

#include <glm/glm.hpp>

struct Particles
{
    int Count;
    float Alingment;    // steer towards the average heading of local flockmates
    float Cohesion;     // steer to move toward the average position (center of mass) of local flockmates
    float Seperation;   // steer to avoid crowding local flockmates

    float MaxSpeed;

    typedef glm::vec3 position_type;

    std::vector<glm::vec3> Direction;
    std::vector<glm::vec3> PrevDirection;
    
    std::vector<glm::vec3> Velocities;
    std::vector<glm::vec3> PrevVelocities;

    std::vector<glm::vec3> Positions;
    std::vector<glm::vec3> PrevPositions;

    std::vector<float> Radii;

    // collision radii
    // visual radii
    // search radii

    // values with curr/max
    // life
    // 

    Path Constraint;

    Particles();
    void init(int count);
    void update(float dt);
    void shutdown();

protected:
    glm::vec3 alingmentSteering(int curr_id);
    glm::vec3 cohesionSteering(int curr_id);
    glm::vec3 seperationSteering(int curr_id);
};

#endif //PARTICLE_H_