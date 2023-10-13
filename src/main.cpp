#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>

#include "ShaderProg/ShaderProg.h"
#include <VBO/VBO.h>
#include <VAO/VAO.h>
#include <VAO/VBLayout.h>
#include <EBO/EBO.h>
#include <Renderer/Renderer.h>
#include <algorithm>
#include <Camera/Camera.h>
#include <glm/glm.hpp>
#define _USE_MATH_DEFINES
#include <cmath>
#include <random>


#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "implot.h"
#include "implot_internal.h"


#include <Points/Points.hpp>
#include <Point/Point.hpp>

//#define REAL_DATA

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const char* glsl_version = "#version 330";

const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

extern const float maxCoord = 10.f; 
extern const float minCoord = 0.f;

std::vector<glm::vec3>::size_type classNum = 3;

bool open = true;

std::vector<glm::vec3> vectors{classNum};
float learningRate{1.0f};

void randomFill();

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED , GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(10, 10, "", NULL, NULL);
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
    
    ShaderProg base("../../shaders/Base.vs", "../../shaders/Base.fs");
    base.Use();
    glUniform1f(base.GetLocation("maxCoord"), maxCoord);


    glPointSize(4);
    glClearColor(0.f, 0.f, 0.f, 0.0f);
    auto prevCNum = 0;
    bool solve = false;
    while (!glfwWindowShouldClose(window))  
    {
        processInput(window);
        Renderer::Clear();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui::ShowDemoWindow();
        
        ImGui::Begin("Lab 4", &open, ImGuiWindowFlags_NoResize);
        ImGui::SetWindowSize(ImVec2(SCR_WIDTH, SCR_HEIGHT));
        ImGui::SetWindowFontScale(2);
        ImGui::SliderInt("Classes to divide", (int*)&classNum, 2, 12000);

        if(prevCNum != classNum)
        {
            vectors.resize(classNum);
            for(int i  = prevCNum; i < classNum; ++i)
            {
                vectors[i][2] = 1;
            }
            prevCNum = classNum;
        }
        ImGui::BeginChild(1, ImVec2(600, 200), true);
            int ind = 1;
            for(auto& vec : vectors)
            {
                ImGui::InputFloat2((std::to_string(ind) + " vec").c_str(), glm::value_ptr(vec));
                ++ind;
            }
        ImGui::EndChild();   
        ImGui::SameLine();
        if(ImGui::Button("Generate"))   
        {
            randomFill();
        }


        ImGui::SliderFloat("Learning rate", &learningRate, 0.00001f, 10.0f);

        static std::vector<glm::vec3> weights{classNum};
        solve = ImGui::Button("Solve", ImVec2(200, 50));
        if(solve)
        {
            weights.clear();
            bool changes = true;
            int steps = 0;
            while (changes && steps < 1000)
            {
                weights.resize(classNum);
                changes = false;
                for(int i = 0; i < classNum; ++i)
                {
                    bool cycleChange = false; 
                    std::vector<float> dotProducts;
                    dotProducts.resize(classNum);
                    for(int j = 0; j < classNum; ++j)
                        dotProducts[j] = glm::dot(vectors[i], weights[j]);
                    
                    auto ethanol = dotProducts[i];
                    for(int j = 0; j < classNum; ++j)
                    {
                        if(j == i) continue;
                        if(dotProducts[j] >= ethanol)
                        {
                            weights[j] -= learningRate * vectors[i];
                            cycleChange = true;
                        }
                    }
                    if(cycleChange)
                    {
                        weights[i] += learningRate * vectors[i];
                        changes = true;
                    }
                }
                ++steps;
            } 
 

        }
        
        ImGui::BeginChild(2, ImVec2(600, 200), true);
        ind = 1;
        for(auto& vec : weights)
        {
            ImGui::InputFloat3((std::to_string(ind) + " vec").c_str(), glm::value_ptr(vec));
            ++ind;
        }
        ImGui::EndChild(); 
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
        open = false;
}

void randomFill()
{
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<float> dis(minCoord, maxCoord);

    for(auto& vec : vectors)
    {
        vec[0] = dis(gen);
        vec[1] = dis(gen);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}