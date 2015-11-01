#pragma once

#include <vector>
#include "glm/glm.hpp"

struct Path
{
public:
    std::vector<float> Radii;
    std::vector<glm::vec3> Points;
    uint32_t Count; 

    void shutdown();
};