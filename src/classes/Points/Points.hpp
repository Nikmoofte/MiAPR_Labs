#ifndef POINTS_HPP
#define POINTS_HPP

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
    glm::vec2& operator[](size_t index) {return data[index]; buffilled = false;}
    const glm::vec2& operator[](size_t index) const {return data[index];}
    void randomFill();
    void randomFill(size_t index);
    void fillBuffer();
    friend class Renderer;
private:
    auto getData() {return data.front();}
    bool buffilled = false;
    std::vector<glm::vec2> data;
    glm::vec3 color;
    VBO vb;
    VAO va;
};

#endif