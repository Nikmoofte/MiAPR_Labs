#include "Points.hpp"

#include <random>

#include "VAO/VBLayout.h"

extern const float maxCoord; 
extern const float minCoord;

Points::Points(size_t numOfPoints) : data(numOfPoints), vb(), va()
{
    VBLayout lo;
    lo.Push<float>(2);
    va.setLayout(vb, lo);
}

Points::Points(): data(), vb(), va()
{
    VBLayout lo;
    lo.Push<float>(2);
    va.setLayout(vb, lo);
}

void Points::setColor(const glm::vec3 &color)
{
    this->color = color;
}

void Points::randomFill(size_t index)
{
    if(index > data.size())
        return;
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<float> dis(minCoord, maxCoord);
    for(auto beg = data.begin() + index, end = data.end(); beg < end; ++beg)
    {
        beg->x = dis(gen);
        beg->y = dis(gen);
    }

    buffilled = false;
}

void Points::randomFill()
{
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<float> dis(minCoord, maxCoord);

    for(auto& point : data)
    {
        point.x = dis(gen);
        point.y = dis(gen);
    }
    buffilled = false;
}

void Points::normalFill(float mathExpect, float deviation)
{
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::normal_distribution<float> dis(mathExpect, deviation);


    for(auto& point : data)
    {
        point.x = dis(gen);
        point.y = dis(gen);
    }
    buffilled = false;
}

void Points::fillBuffer()
{
    if(data.size() > 0)
    {
        vb.Bind();
        vb.SetBufferData(data.size() * sizeof(glm::vec2), &data.front(), GL_STATIC_DRAW);
        buffilled = true;
    }
}
