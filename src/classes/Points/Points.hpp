#ifndef POINTS_HPP
#define POINTS_HPP

#include <list>
#include <vector>
#include <VAO/VAO.h>
#include <VBO/VBO.h>
#include <glm/glm.hpp>


struct Points
{
    Points(size_t numOfPoints);
    Points();
    void Push(glm::vec2 pos) 
    {
        data.push_back(pos); 
        buffilled = false;
    }
    void clear() 
    {
        data.clear(); 
        buffilled = false;
    }
    void resize(size_t newSize) { data.resize(newSize); buffilled = false;}
    size_t getSize() const {return data.size();}
    void setColor(const glm::vec3& color);
    void randomFill();

    void normalFill(float mathExpect, float deviation);
    double getMathExpectence();
    double getDeviation(double mathExpectence);
    double getDeviation();

    void fillBuffer();
    auto& getList() {return data;}
    friend class Renderer;
private:
    auto getData() {return data.front();}
    bool buffilled = false;
    std::list<glm::vec2> data;
    glm::vec3 color;
    VBO vb;
    VAO va;
};

#endif