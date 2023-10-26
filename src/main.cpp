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

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


extern const float maxCoord = 10.f; 
extern const float minCoord = 0.f; 

double delta = 0.0000001;

unsigned pointsNum = 1000;

std::vector<glm::vec3> colors = {
    {1.f, 0.f, 0.f},
    {0.f, 1.f, 0.f},
    {0.f, 0.f, 1.f},
    {1.f, 1.f, 0.f},
    {1.f, 0.f, 1.f},
    {0.f, 1.f, 1.f},
    {0.75f, 0.75f, 0.25f},
    {0.75f, 0.25f, 0.75f},
    {0.25f, 0.75f, 0.75f},
    {0.50f, 0.25f, 0.50f}
    };

struct normalData
{
    double x0;
    double h;
    double mathExpect;
    double deviation;
};

inline double normalFunc(double x, double mathExpect, double deviation);
ImPlotPoint normalFunc(int idx, void* data);
double simpson_rule(double a, double b, int n, double mathExpect, double deviation);
glm::vec2 intersection(const normalData& data1, const normalData& data2, double delta);


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
    
    ShaderProg base("../../shaders/Base.vs", "../../shaders/Base.fs");
    base.Use();
    glUniform1f(base.GetLocation("maxCoord"), maxCoord);

    normalData data1{0.0, 0.01, 4.0, 1.0};
    normalData data2{0.0, 0.01, 7.0, 0.5};

    Points class1{2}, class2{2};
    class1.setColor(colors[0]);
    class2.setColor(colors[1]);

    class1.normalFill(data1.mathExpect, data1.deviation);
    class2.normalFill(data2.mathExpect, data2.deviation);


    glPointSize(10);
    glClearColor(0.f, 0.f, 0.f, 0.0f);
    bool open = true;
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        Renderer::Clear();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui::ShowDemoWindow();
        //ImPlot::ShowDemoWindow();
        ImGui::Begin("Lab 4", &open, ImGuiWindowFlags_NoResize);
        ImGui::SetWindowSize(ImVec2(SCR_WIDTH, SCR_HEIGHT));
        static bool change = true;
        if(ImGui::Button("Redestrebute", ImVec2(200, 50)))
        {
            class1.normalFill(data1.mathExpect, data1.deviation);
            class2.normalFill(data2.mathExpect, data2.deviation);
            change = true;
        }
        ImPlot::BeginPlot("Plot");
        auto* data = &class1.getData();
        ImPlot::PlotScatter("data 1", (float*)data, (float*)data + 1, class1.getSize(), 0, 0, sizeof(glm::vec2));
        data = &class2.getData();
        ImPlot::PlotScatter("data 2", (float*)data, (float*)data + 1, class2.getSize(), 0, 0, sizeof(glm::vec2));
        
        static bool calc = 0;
        calc ^= ImGui::Button("Calculate", ImVec2(200, 50));
        if(calc)
        {
            static glm::vec4 finalCoefs{0.0f};
            // class1[0] = glm::vec2(-1.7, 3.1); 
            // class1[1] = glm::vec2(3.1, 4.2); 

            // class2[0] = glm::vec2(6.4, 7); 
            // class2[1] = glm::vec2(7.1, 7.1); 
            if(change)
            {
                glm::vec4 coefs{0.0f};
                auto func = [&coefs](const glm::vec2& point){ return coefs.x + coefs.y * point.x + coefs.z * point.y + coefs.w * point.x * point.y; }; 
                float p = 1;
                bool temp = true;
                bool working = true;
                while(working)
                {
                    working = false;
                    for(int i = 0; i < class1.getSize(); ++i)
                    {
                        auto& point = class1[i];
                        glm::vec4 correction{p, p * 4 *point.x, p * 4 * point.y, p * 16 * point.x * point.y};
                        if(func(point) <= 0)
                        {
                            coefs += correction;
                            working = true;
                        }
                    }
                    for(int i = 0; i < class2.getSize(); ++i)
                    {
                        auto& point = class2[i];
                        glm::vec4 correction{p, p * 4 *point.x, p * 4 * point.y, p * 16 * point.x * point.y};
                        if(func(point) > 0)
                        {
                            coefs -= correction;
                            working = true;
                        }
                    }
                    
                }
                
                finalCoefs = coefs;
            }
            struct _data
            {
                double x0;
                double h;
                glm::vec4 coef;
            } data{-3, 0.00001, finalCoefs};
            auto func = [](int idx, void* data) -> ImPlotPoint
            {
                auto& ndata = *(_data*)data;  
                double x = ndata.x0 + idx * ndata.h;
                return ImPlotPoint(x, -(ndata.coef.x + ndata.coef.y * x) / (ndata.coef.z + ndata.coef.w * x));
            }; 
            ImPlot::PlotLineG("dev func", func, &data, 900000, ImPlotLineFlags_Segments);
            data.x0 = 6;
            ImPlot::PlotLineG("dev func", func, &data, 300000, ImPlotLineFlags_Segments);
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

ImPlotPoint normalFunc(int idx, void *data)
{
    auto ndata = (normalData*)data;
    double x{ndata->x0 + idx * ndata->h}, y{};

    y = normalFunc(x, ndata->mathExpect, ndata->deviation);
    return ImPlotPoint(x, y);
}

double simpson_rule(double a, double b, int n, double mathExpect, double deviation)
{
    double h = (b - a) / n;

    // Internal sample points, there should be n - 1 of them
    double sum_odds = 0.0;
    for (int i = 1; i < n; i += 2)
    {
        sum_odds += normalFunc(a + i * h, mathExpect, deviation);
    }
    double sum_evens = 0.0;
    for (int i = 2; i < n; i += 2)
    {
        sum_evens += normalFunc(a + i * h, mathExpect, deviation);
    }

    return (normalFunc(a, mathExpect, deviation) + normalFunc(b, mathExpect, deviation) + 2 * sum_evens + 4 * sum_odds) * h / 3;
}

inline double normalFunc(double x, double mathExpect, double deviation)
{
    static constexpr double sqrt2pi = M_2_SQRTPI * M_SQRT1_2 / 2; // 1/sqrt(2pi)
    return sqrt2pi / deviation * exp(-0.5f * pow((x - mathExpect) / deviation, 2));
}



void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}