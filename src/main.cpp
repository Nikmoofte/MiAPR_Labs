#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <stack>

#include <glm/glm.hpp>
#define _USE_MATH_DEFINES

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "implot.h"
#include "implot_internal.h"


//#define REAL_DATA

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void randomFill(std::vector<std::vector<float>>& vec);
void vecOut(const std::vector<std::vector<float>> &vec);
void processInput(GLFWwindow *window);

std::string toPosfix(const std::string& str)
{
    std::unordered_map<char, int> priority
    {
        {'(', 0}, 
        {')', 1}, 
        {'=', 2}, 
        {'+', 3}, 
        {'-', 3}, 
        {'*', 3}, 
    };

    std::stringstream ss("("+ str + ")");
    std::istream_iterator<char> beg(ss), end;
    std::string result;
    std::stack<char> stack;

    while(beg != end)
    {
        char symb = *beg;
        if(!priority.contains(symb))
        {
            result += symb;
            ++beg;
        }
        else
        {
            if(symb == '(');
            else while(priority[symb] <= priority[stack.top()])
            {
                result += stack.top();
                stack.pop();
            }
            if(!(symb == ')'))
                stack.push(symb);
            else
                stack.pop();
            ++beg;
        }
    }
    return result;
}



// settings
const char* glsl_version = "#version 330";

constexpr float delta = 0.0001;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


extern const float maxCoord = 10.f; 
extern const float minCoord = 0.f; 

namespace PDL
{
    struct Line
    {
        Line(const glm::vec2& origin, const glm::vec2& dir, float length) : from(origin), dir(glm::normalize(dir)), length(length){}
        Line(const Line& other) : from(other.from), dir(other.dir), length(other.length){}
        Line() {}

        bool operator==(const Line& other) const
        {
            return from == other.from && dir == other.dir && length == other.length;
        }

        glm::vec2 getEnd()
        {
            return from + dir * length;
        }
        void setEnd(glm::vec2 end)
        {
            end -= from;
            length = glm::length(end);
            dir = glm::normalize(end);
        }

        void moveEndTo(const glm::vec2& end)
        {
            from = end - dir * length;
        }
        void moveTo(const glm::vec2& origin)
        {
            from = origin;
        }

        float getLength() const
        {
            return length;
        }
        void setLength(float _length)
        {
            length = _length;
        }

        void setDir(const glm::vec2& _dir)
        {
            dir = glm::normalize(_dir);
        }
        const glm::vec2& getDir() const
        {
            return dir;
        }
        
        void setOrigin(const glm::vec2& _from)
        {
            from = _from;
        }
        const glm::vec2& getOrigin() const
        {
            return from;
        }



        std::string toString()
        {
            std::stringstream result("");
            result << "Line: \n" <<
                    "{\n" <<
                    "origin: ("<<  from.x << " " << from.y << ")\n" <<
                    "dir: (" << dir.x << " " << dir.y << ")\n" <<
                    "length: ("<< length << ")" << 
                    "}\n";  
            return result.str();
        }
    private:
        glm::vec2 from, dir;
        float length;
    };

    struct HorizontalLine : public Line
    {
        HorizontalLine(float dx) : Line({0, 0}, {dx, 0}, 1){}
        //inline HorizontalLine(const Line& other) : Line(other){}
    };

    struct VertialLine : public Line
    {
        VertialLine(float dy) : Line({0, 0}, {0, dy}, 1){}
    };

    struct DiagonalLine : public Line
    {
        DiagonalLine(float dx, float dy) : Line({0, 0}, {dx, dy}, std::sqrt(2.0f)){}
    };

    class Lines
    {
    public:
        Lines() = default;
        Lines(Line& line) : _data({line}){}
        Lines(Line&& line) : _data({std::move(line)}){}
        Lines(Lines&& other) : _data(std::move(other._data)){}
        Lines(Lines& other) : _data(other._data){}
        Lines(std::string line);

        Lines& operator+=(const Lines& other);
        Lines& operator-=(const Lines& other);
        Lines& operator*=(const Lines& other);
        bool empty() const;

        Lines& operator=(Lines&& other)
        {
            _data = std::move(other._data);
            first = true;
            return *this;
        }
        Lines& operator=(const Lines& other)
        {
            _data = other._data;
            first = true;
            return *this;
        }
        bool operator==(const Lines& other) const
        {
            if(_data.size() != other._data.size())
                return false;
            bool result = true;
            for(size_t i = 0; i < _data.size(); ++i)
                result &= _data[i] == other._data[i];
            return result;
        }

        void Plot();

        std::string toString();

        friend struct std::hash<Lines>;
        
    private:
        bool first = true;
        size_t copy(const Lines& other);
        size_t reverseCopy(const Lines& other);


        std::vector<Line> _data;
    };
    
    std::unordered_map<char, Lines> terminals;

    std::unordered_map<char, Lines&(Lines::*)(const Lines&)> operators{
        {'+', &Lines::operator+=},
        {'-', &Lines::operator-=},
    };

    Lines::Lines(std::string line)
    {
        if(line.size() < 3)
        {
            _data = terminals[line[0]]._data;
            return;
        }
        line = toPosfix(line);
        size_t pos;
        size_t tempNonterminal = 0;
        while((pos = line.find_first_of("+-*/=")) != std::string::npos)
        {
            if(line[pos] == '=')
            {
                terminals[line[pos - 2]] = terminals[line[pos - 1]];
                line.replace(pos - 2, 3, std::string(1, line[pos - 2]));
                if(line.size() == 1)
                    tempNonterminal = line[0] + 1;
            }
            else
            {
                assert(tempNonterminal < 40 && "Too many nonterminals");

                auto res = terminals[line[pos - 2]];
                auto secondOperand = terminals[line[pos - 1]];

                (res.*operators[line[pos]])(secondOperand);
                terminals[tempNonterminal] = res;

                line.replace(pos - 2, 3, std::string(1, tempNonterminal));

                ++tempNonterminal;
            }
        }
        copy(terminals[tempNonterminal - 1]);
    }
    Lines& Lines::operator+=(const Lines& other)
    {
        auto prevSize = copy(other);
        for(size_t i = prevSize; i < _data.size(); ++i)
            _data[i].moveTo(_data[i - 1].getEnd());
        
        return *this;
    }
    Lines& Lines::operator-=(const Lines& other)
    {
        auto prevSize = reverseCopy(other);
        for(size_t i = prevSize; i < _data.size(); ++i)
        {
            _data[i].moveTo(_data[i - 1].getEnd());
            _data[i].setDir(-_data[i].getDir());
        }
        
        
        return *this;
    }
    Lines& Lines::operator*=(const Lines& other)
    {
        auto prevSize = copy(other);

        _data[prevSize].setOrigin(_data[0].getOrigin());
        for(size_t i = prevSize + 1; i < _data.size() - 1; ++i)
            _data[i].setOrigin(_data[i - 1].getEnd());
        _data[_data.size() - 1].setEnd(_data[prevSize - 1].getEnd());

        return *this;
    }
    bool Lines::empty() const
    {
        return _data.empty();
    }
    void Lines::Plot()
    {
        if(_data.empty())
            return;
        if(first)
        {
            _data.push_back(Line{{0, 0}, {0, 0}, 0});
            _data[_data.size() - 1].setOrigin(_data[_data.size() - 2].getEnd());
            first = false;
        }
        float* ptr = (float*)&_data.front();
        ImPlot::PlotLine("Plot", ptr, ptr + 1, _data.size(), 0, 0, sizeof(Line));
    }

    std::string Lines::toString()
    {
        std::string result;
        for(auto& elem : _data)
            result += elem.toString();
        return result;
    }
    size_t Lines::copy(const Lines& other)
    {
        auto prevSize = _data.size(); 
        _data.resize(prevSize + other._data.size());
        std::copy(other._data.begin(), other._data.end(), _data.begin() + prevSize);
        return prevSize;
    }
    size_t Lines::reverseCopy(const Lines& other)
    {
        auto prevSize = _data.size();
        _data.resize(prevSize + other._data.size());
        std::copy(other._data.rbegin(), other._data.rend(), _data.begin() + prevSize);
        return prevSize;
    }
}

namespace std
{
    template<>
    struct hash<PDL::Lines>
    {
        size_t operator()(const PDL::Lines& lines) const
        {
            size_t result = 0;
            for(auto& elem : lines._data)
            {
                result += std::hash<float>()(elem.getOrigin().x);
                result += std::hash<float>()(elem.getOrigin().y);
                result += std::hash<float>()(elem.getDir().x);
                result += std::hash<float>()(elem.getDir().y);
                result += std::hash<float>()(elem.getLength());
            }
            return result;
        }
    };
}

int main()
{
    PDL::terminals['a'] = PDL::Lines(PDL::HorizontalLine(1.0f));
    PDL::terminals['b'] = PDL::Lines(PDL::VertialLine(1.0f));
    PDL::terminals['c'] = PDL::Lines(PDL::DiagonalLine(1.0f, 1.0f));
    PDL::terminals['d'] = PDL::Lines(PDL::DiagonalLine(1.0f, -1.0f));

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED , GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(10, 10, "Statistic distribution", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    std::unordered_map<std::string, std::string> objects;
    objects[toPosfix("a + b - a - b")] = "square";
    objects[toPosfix("c + d - c - d")] = "rohmbus";
    objects[toPosfix("a + b + d - c + b")] = "arrow";

    std::unordered_map<PDL::Lines, std::string> lObjects;
    lObjects[PDL::Lines("a + b - a - b")] = "square";
    lObjects[PDL::Lines("c + d - c - d")] = "rohmbus";
    lObjects[PDL::Lines("a + b + d - c + b")] = "arrow";
    lObjects[PDL::Lines("a")] = "horizontal line";
    lObjects[PDL::Lines("b")] = "vertical line";    
    lObjects[PDL::Lines("c")] = "diagonal line";
    lObjects[PDL::Lines("d")] = "diagonal line";
    

    PDL::Lines result;

    bool open = true;
    PDL::Lines toCheck;
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //ImGui::ShowDemoWindow();
        //ImPlot::ShowDemoWindow();
        
        ImGui::Begin("Lab 7", &open, ImGuiWindowFlags_NoResize);
        ImGui::SetWindowSize({SCR_WIDTH, SCR_HEIGHT});
        static char buff[256]{};
        ImGui::InputTextMultiline("Expression", buff, 256, {640, 200});
        if(ImGui::Button("Solve", {100, 30}) && buff[0] != '\0')
        {
            static char tempNonterminal; 
            toCheck = PDL::Lines();
            std::string str{buff};
            std::vector<std::string> lines;
            {
            std::istringstream iss(str);
                std::string line;
                while (std::getline(iss, line, '\n')) {
                    lines.push_back(line);
                }
            }
            
            for(auto& line : lines)
            {
                enum actions
                {
                    to_plot,
                    to_check
                } action;
                std::string lowerStr = line;
                std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
                size_t pos; 
                if((pos = lowerStr.find("check")) != std::string::npos)
                {
                    action = actions::to_check;
                    line.erase(pos, 6);
                }
                else
                    action = actions::to_plot;

                switch(action)
                {
                    case actions::to_plot:
                    {
                        result = PDL::Lines(line);
                        break;
                    }
                    case actions::to_check:
                    {
                        toCheck = PDL::Lines(line);
                    }
                }
            }
        }
        // if(terminals.contains('#'))
        //     ImGui::Text(terminals['#'].toString().c_str());
        if(!toCheck.empty())
        {
            bool found = false;
            for(auto& elem : lObjects)
            {
                if(elem.first == toCheck)
                {
                    ImGui::Text("There is %s", elem.second.c_str());
                    found = true;
                }
            }
            if(!found)
                ImGui::Text("This is no kwown object");
        }
        if(ImPlot::BeginPlot("Plot"))
        {
            result.Plot();
            ImPlot::EndPlot();
        }
        ImGui::End();
       

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        glfwSetWindowShouldClose(window, !open);

    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}


void randomFill(std::vector<std::vector<float>> &vec)
{
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<float> dis(0, 100.0);
    for(auto& arr : vec)
        for(auto& elem : arr)
            elem = dis(gen);
}

void vecOut(const std::vector<std::vector<float>> &vec)
{
    for(auto& arr : vec)
    {
        std::cout << "[ ";
        for(auto elem : arr)
            std::cout <<  std::setw(7) << std::setprecision(4) << std::setfill(' ') << elem << ", " << '\t';
        std::cout << " ]" << "\n";     
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
