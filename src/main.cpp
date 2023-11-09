#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <random>
#include <memory>
#include <cmath>
#include <cfloat>
#include <algorithm>
#include <string>

#include <glm/glm.hpp>
#define _USE_MATH_DEFINES

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "implot.h"
#include "implot_internal.h"


//#define REAL_DATA

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void delTrash(std::vector<std::vector<float>> &vec, const glm::vec2& pos, bool (*compareFunc)(float, float));
void vecOut(const std::vector<std::vector<float>> &vec);
void processInput(GLFWwindow *window);

// settings
const char* glsl_version = "#version 330";

constexpr float delta = 0.0001;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


extern const float maxCoord = 10.f; 
extern const float minCoord = 0.f; 

auto minFunc = [](float f, float s) { return f < s; };
auto maxFunc = [](float f, float s) { return f > s; };

struct child
{
    glm::vec2 first{};
    glm::vec2 second{};
    float val;
};

int hierarchyFind(const std::vector<child>&hierarchy, float);

size_t pointNum = 10;

void randomFill(std::vector<std::vector<float>>& vec);
glm::vec2 find(std::vector<std::vector<float>>& vec, bool (*compareFunc)(float, float));

int main()
{
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


    std::vector<std::vector<float>> dists(pointNum);
    for(auto& elem : dists) elem.resize(pointNum);
    randomFill(dists);
    for(int i = 0; i < pointNum; ++i)
    {
        dists[i][i] = 0.0f;
        for(int j = 0; j < i; ++j)
            dists[i][j] = dists[j][i];
    }


    glPointSize(10);
    glClearColor(0.f, 0.f, 0.f, 0.0f);
    bool open = true;
    std::vector<child> hierarchy;
    auto copyDists = dists;
    auto prevPointNum = pointNum;
    vecOut(copyDists);
    std::cout << std::endl;
    for(auto& elem : hierarchy)
    {
        std::cout << elem.first.x << ' ' << elem.second.x << '\n'; 
    }
    std::cout << std::endl;
    bool reFill = false;
    bool cmprFunc = false;
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //ImGui::ShowDemoWindow();
        //ImPlot::ShowDemoWindow();
        ImGui::Begin("Lab 4", &open, ImGuiWindowFlags_NoResize);
        ImGui::SetWindowSize(ImVec2(SCR_WIDTH, SCR_HEIGHT));
        
        ImGui::SliderInt("size", reinterpret_cast<int*>(&pointNum), 2, 10);

        size_t tableSize = pointNum + 1;
        if (ImGui::BeginTable("Input", tableSize))
        {
            ImGui::TableNextRow();
            for(int j = 0; j < tableSize; ++j)
            {
                ImGui::TableSetColumnIndex(j);
                ImGui::Text("x%d", j);
            }
            for (int i = 1; i < tableSize ; ++i)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("x%d", i);
                for (int j = 1; j < tableSize; ++j)
                {
                    ImGui::TableSetColumnIndex(j);
                    ImGui::InputFloat((std::string("##") + std::to_string(i) + std::to_string(j)).c_str(), &(dists[i - 1][j - 1]), 0, 100.0);
                }
            }
            ImGui::EndTable();

            static bool prevState = cmprFunc;
            ImGui::Checkbox("Max", &cmprFunc);
            reFill = prevState != cmprFunc;
            prevState = cmprFunc;

            ImGui::SameLine();
            if(reFill = ImGui::Button("Regenerate", ImVec2(100, 30)))
            {
                randomFill(dists);
                for(int i = 0; i < pointNum; ++i)
                {
                    dists[i][i] = 0.0f;
                    for(int j = 0; j < i; ++j)
                        dists[i][j] = dists[j][i];
                }
            }

        }

        static auto size = copyDists.size();
        if(pointNum != prevPointNum || reFill)
        {
            copyDists = dists;
            copyDists.resize(pointNum);
            for(auto& arr : copyDists)
                arr.resize(pointNum);
            hierarchy.clear();

            size = pointNum;
            prevPointNum = pointNum;
        }

        while(size > 1)
        {
            glm::vec2 anchor;
            if(cmprFunc)
                anchor = find(copyDists, maxFunc);
            else
                anchor = find(copyDists, minFunc);
            
            child a;
            int ind;
            if((ind = hierarchyFind(hierarchy, anchor.x)) != -1)
            {
                auto& chld = hierarchy[ind];
                a.first.x = (chld.first.x + chld.second.x) / 2.0f;
                a.first.y = chld.val;    
            }
            else
                a.first.x = anchor.x;

            if((ind = hierarchyFind(hierarchy, anchor.y)) != -1)
            {
                auto& chld = hierarchy[ind];
                a.second.x = (chld.first.x + chld.second.x) / 2.0f;
                a.second.y = chld.val;    
            }
            else
                a.second.x = anchor.y;
            a.val = copyDists[anchor.y][anchor.x];

            hierarchy.push_back(a);
            if(cmprFunc)
                delTrash(copyDists, anchor, maxFunc);
            else
                delTrash(copyDists, anchor, minFunc);
            --size;

            vecOut(copyDists);
            std::cout << std::endl;
            for(auto& elem : hierarchy)
            {
                std::cout << elem.first.x << ' ' << elem.second.x << ' ' << elem.val << '\n'; 
            }
            std::cout << std::endl;
        }


        ImPlot::BeginPlot("Plot");
        int ind = 0;
        for(auto& elem : hierarchy)
        {
            std::string lbl("a");
            lbl[0] += ind;

            float xb[] = {elem.first.x, elem.first.x, elem.second.x, elem.second.x}; 
            float yb[] = {elem.first.y, elem.val, elem.val, elem.second.y};               

            ImPlot::PlotLine(lbl.c_str(), xb, yb, 4);
            ++ind;
        }
        ImPlot::EndPlot();
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

int hierarchyFind(const std::vector<child> &hierarchy, float a)
{
    int res = -1;
    int temp = 0;
    for(auto& elem : hierarchy)
    {
        if(std::abs(elem.second.x - a) <= delta ||
           std::abs(elem.first.x - a) <= delta) 
        {
            res = temp;
            a = (elem.first.x + elem.second.x) / 2.0f;
        }
        ++temp;
    }
    return res;
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

glm::vec2 find(std::vector<std::vector<float>> &vec, bool (*compareFunc)(float, float))
{
    glm::vec2 res;
    bool end = false;
    for(int i = 0; i < vec.size() && !end; ++i)
    {
        for(int j = i + 1; j < vec.size() && !end; ++j)
        {
            res = glm::vec2(j, i);
            if(vec[i][j] > 0)
                end = true;
        }
    }

    for(size_t i = 0; i < vec.size(); ++i)
    {
        for(size_t j = i + 1; j < vec.size(); ++j)
        {
            if(vec[i][j] <= 0) 
                continue;
            if(compareFunc(vec[i][j], vec[res.y][res.x]))
                res = glm::vec2(j, i);
        }
    }

    return res;
}


//no deliting in the [pos.y] class place data from union, [pos.x] now is -1 every where
void delTrash(std::vector<std::vector<float>> &vec, const glm::vec2& pos, bool (*compareFunc)(float, float))
{
    for(int i = 0; i < vec.size(); ++i)
    {
        auto maxVal = compareFunc(vec[pos.x][i], vec[pos.y][i]) ? vec[pos.x][i] : vec[pos.y][i];
        //[pos.y] class place data from union
        vec[pos.y][i] = maxVal;
        vec[i][pos.y] = maxVal;
        
        vec[pos.x][i] = -1.0f;
        vec[i][pos.x] = -1.0f;
    }

    
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
