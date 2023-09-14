#ifndef SHADERPROG_H
#define SHADERPROG_H

#include <glad/glad.h>
#include <cstddef>
#include <string>
#include <unordered_map>


class ShaderProg
{
public:
    ShaderProg(const std::string& vertexFilepath, const std::string& fragmentFilepath);
    ShaderProg(const ShaderProg&);
    ~ShaderProg();

    void Use() const;
    void UnUse() const;

    GLint GetLocation(const std::string& name);
private:
    unsigned CompileShader(GLenum type, const std::string& source);
    unsigned CreateProgram(const std::string& vertexSource, const std::string& fragmentSource);
    std::string GetSourceFromFile(const std::string& filePath);
    std::unordered_map<std::string, int> UniformLocCache;
    unsigned id;
};

#endif