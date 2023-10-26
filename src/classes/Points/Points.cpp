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

    float height = 0.0f;
    for(auto& point : data)
    {
        point.x = dis(gen);
        point.y = dis(gen);
        // point.y = height;
        // height += 10.0f / getSize();
    }
    buffilled = false;
}

double Points::getMathExpectence()
{
    double result = 0.0;
    for(auto& point : data)
    {
        result += point.x;
    }
    result /= data.size();

    return result;
}

double Points::getDeviation(double mathExpectence)
{
    double result = 0.0;
    for(auto& point : data)
    {
        result += pow(point.x - mathExpectence, 2);
    }
    result /= data.size();

    return result;
}

double Points::getDeviation()
{
    return getDeviation(getMathExpectence());
}

void Points::fillBuffer()
{
    std::vector<glm::vec2> vec(data.begin(), data.end());

    if(data.size() > 0)
    {
        vb.Bind();
        vb.SetBufferData(data.size() * sizeof(glm::vec2), &vec.front(), GL_STATIC_DRAW);
        buffilled = true;
    }
}
