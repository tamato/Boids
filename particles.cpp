#include <iostream>

#include "particles.h"

#include "glm/gtc/constants.hpp"
#include "glm/gtc/random.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/norm.hpp"         // for length2

Particles::Particles()
 : Count(0)
 , Alingment(1000)
 , Cohesion(100)
 , Seperation(5)
 , MaxSpeed(100)
{

}

void Particles::init(int count)
{
    Count = count;
    
    Positions.resize(count);
    PrevPositions.resize(count);
    
    Velocities.resize(count, glm::vec3(0));
    PrevVelocities.resize(count, glm::vec3(0));
    
    Radii.resize(count, 0);

    Direction.resize(count);
    PrevDirection.resize(count, glm::vec3(0));

    float cylinder_center = 50.f;
    Constraint.Count = 100;
    Constraint.Radii.resize(Constraint.Count);
    Constraint.Points.resize(Constraint.Count);
    for (uint32_t i = 0; i < Constraint.Count; ++i){
        float percent = i/float(Constraint.Count);
        float val = percent * glm::two_pi<float>();

        float x = std::cos(val) * cylinder_center;
        float y = std::sin(val) * cylinder_center;

        Constraint.Radii[i] = 5.f * std::max(1.0f-percent, 0.1f);
        Constraint.Points[i] = glm::vec3(x,y,0);
    }

    for (uint32_t i=0; i<count; ++i){
        Positions[i] = Constraint.Points[0] + glm::sphericalRand(Constraint.Radii[0]);;
        PrevPositions[i] = Constraint.Points[0];

        Direction[i] = glm::sphericalRand(.1f);
    }
}

void Particles::update(float dt)
{
    PrevPositions = Positions;
    PrevVelocities = Velocities;
    PrevDirection = Direction;

    // std::cout << ".................................................." << std::endl;
    float flow_influence = 30000.f * dt;
    for (int i=0; i<Count; ++i){
        
        uint32_t pt0, pt1;
        Constraint.nearestPointIDs(pt0, pt1, Positions[i]);
        glm::vec3 flow_direction = Constraint.Points[pt1] - Constraint.Points[pt0];
        float flow_len = glm::length(flow_direction);
        if (flow_len != 0.0f)
            flow_direction /= flow_len;

        glm::vec3 steering_force(0);
        
        glm::vec3 flow = flow_direction * flow_influence;
        glm::vec3 alingment = alingmentSteering(i);
        glm::vec3 seperation = seperationSteering(i);
        glm::vec3 cohesion = cohesionSteering(i);

        steering_force += flow;
        steering_force += alingment * 5.f;
        steering_force += seperation * 5.f;
        steering_force += cohesion * 1.5f;

        // std::cout << "Flow: " << glm::to_string(flow) << std::endl;
        // std::cout << "Alingment: " << glm::to_string(alingment) << std::endl;
        // std::cout << "Seperation: " << glm::to_string(seperation) << std::endl;
        // std::cout << "Cohesion: " << glm::to_string(cohesion) << std::endl;
        // std::cout << "steering_force: " << glm::to_string(steering_force) << std::endl;

        Velocities[i] = steering_force * dt;
        glm::vec3 desired_pos = Positions[i] + Velocities[i];

        float projected_length = glm::dot(Velocities[i], flow_direction);
        glm::vec3 projected_pos = Constraint.Points[pt0] + flow_direction * projected_length;
        glm::vec3 limit = projected_pos - desired_pos;
        float limit_len = glm::length(limit);
        float distance_check = limit_len - Constraint.Radii[i];
        // std::cout << "Distance: " << distance_check << std::endl;
        if (distance_check > 0){
            limit /= limit_len;
            limit *= distance_check;
            // std::cout << "D: " << glm::to_string(desired_pos) << " l: " << glm::to_string(limit) << std::endl;
            // std::cout << "P: " << glm::to_string(Positions[i]) << " V: " << glm::to_string(Velocities[i]) << std::endl;
    
            desired_pos = limit + desired_pos;
            Velocities[i] = desired_pos - Positions[i];
        }

        Positions[i] = desired_pos;
    }

}

void Particles::shutdown()
{
    Count = 0;
    Velocities.clear();
    PrevVelocities.clear();
    Positions.clear();
    PrevPositions.clear();
    Radii.clear();

    Constraint.shutdown();
}

// steer towards the average heading of local flockmates
glm::vec3 Particles::alingmentSteering(int curr_id)
{
    float valSqr = Alingment*Alingment;
    glm::vec3 curr_pos = Positions[curr_id];
    std::vector<int> ids;
    for (int i=0; i<Count; ++i){
        if (i == curr_id) continue;

        glm::vec3 pos = Positions[i];
        float dist = glm::length2(pos - curr_pos);
        if (dist <= valSqr)
            ids.push_back( i );
    }

    glm::vec3 desired_vel(Velocities[curr_id]);
    if (false == ids.empty()){
        for (const auto& id : ids){
            desired_vel += PrevVelocities[id];
        }
        // desired_vel /= float(ids.size());
    }
    glm::vec3 result = desired_vel - Velocities[curr_id];
    return result;
}

// steer to move toward the average position (center of mass) of local flockmates
glm::vec3 Particles::cohesionSteering(int curr_id)
{
    float valSqr = Cohesion*Cohesion;
    glm::vec3 curr_pos = Positions[curr_id];
    std::vector<int> ids;
    for (int i=0; i<Count; ++i){
        if (i == curr_id) continue;

        glm::vec3 pos = Positions[i];
        float dist = glm::length2(pos - curr_pos);
        if (dist <= valSqr)
            ids.push_back( i );
    }

    glm::vec3 center(curr_pos);
    for (const auto& id : ids){
        center += PrevPositions[id];
    }
    center /= float(ids.size());

    glm::vec3 heading = center - curr_pos;

    glm::vec3 desired_vel(Velocities[curr_id]);
    if (false == ids.empty()){
        for (const auto& id : ids){
            desired_vel = PrevPositions[id] - curr_pos;
        }
        desired_vel /= float(ids.size());
    }
    glm::vec3 result = desired_vel - Velocities[curr_id];
    return result;
}

// steer to avoid crowding local flockmates
glm::vec3 Particles::seperationSteering(int curr_id)
{
    float valSqr = Seperation*Seperation;
    glm::vec3 curr_pos = Positions[curr_id];
    std::vector<int> ids;
    for (int i=0; i<Count; ++i){
        if (i == curr_id) continue;

        glm::vec3 pos = Positions[i];
        float dist = glm::length2(pos - curr_pos);
        if (dist <= valSqr)
            ids.push_back( i );
    }

    glm::vec3 desired_vel(Velocities[curr_id]);
    if (false == ids.empty()){
        for (const auto& id : ids){
            desired_vel += curr_pos - PrevPositions[id];
        }
    }
    glm::vec3 result = desired_vel - Velocities[curr_id];
    return result;
}