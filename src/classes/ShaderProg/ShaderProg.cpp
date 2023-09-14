#include "ShaderProg.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>

ShaderProg::ShaderProg(const std::string &vertexFilepath, const std::string &fragmentFilepath)
{
    id = CreateProgram(GetSourceFromFile(vertexFilepath), GetSourceFromFile(fragmentFilepath));
}

ShaderProg::ShaderProg(const ShaderProg &other)
{
    this->id = other.id;
}

ShaderProg::~ShaderProg()
{
    glDeleteProgram(this->id);
}

void ShaderProg::Use() const
{
    glUseProgram(this->id);
}

void ShaderProg::UnUse() const
{
    glUseProgram(0);
}

GLint ShaderProg::GetLocation(const std::string &name)
{ 
    if(UniformLocCache.find(name) != UniformLocCache.end())
        return UniformLocCache[name];

    GLint location = glGetUniformLocation(id, name.c_str());
    if(location == -1)
        std::cerr << "Uniform " + name + " not found!" << std::endl;
    else
        UniformLocCache[name] = location;
    return location;
}

unsigned ShaderProg::CompileShader(GLenum type, const std::string &source)
{
    unsigned int Shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(Shader, 1, &src, NULL);
    glCompileShader(Shader);
    int result;
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &result);
    if(!result)
    {
        glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &result);
        char* log = new char[result];
        glGetShaderInfoLog(Shader, result, &result, log);
        std::string message((type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"));
        message +=  "shader failed to compile: \n";
        message += log;
        message += source;    

        throw std::runtime_error(message);
    }
    
    return Shader;
}

unsigned ShaderProg::CreateProgram(const std::string &vertexSource, const std::string &fragmentSource)
{
    unsigned int shaderProgram = glCreateProgram();
    
    unsigned vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
    
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);

    int result;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &result);
    if(!result)
    {
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &result);
        char* log = new char[result];
        glGetProgramInfoLog(shaderProgram, result, &result, log);
        std::string message("Program failed to link: \n");
        message += log;
        message += vertexSource + "\n\n";    
        message += fragmentSource + "\n\n";
        throw std::runtime_error(message);
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return shaderProgram;
}

std::string ShaderProg::GetSourceFromFile(const std::string &filePath)
{
    std::ifstream in(filePath);
    std::stringstream sIn;
    sIn << in.rdbuf();
    return sIn.str();
}
