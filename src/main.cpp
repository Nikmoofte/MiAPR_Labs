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
void processInput(GLFWwindow *window, Point2D&);

// settings
const char* glsl_version = "#version 330";

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

#ifdef REAL_DATA
extern const float maxCoord = 140.f; 
#else
extern const float maxCoord = 10.f; 
#endif
extern const float minCoord = 0.f; 

unsigned coresNum = 6;

#ifdef REAL_DATA
unsigned pointsNum = 201;
#else
unsigned pointsNum = 1000;
#endif

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

inline void distribute(const Points& points, const Points& cores, std::vector<Points>& vec);
inline void setColors(std::vector<Points>& vec);
inline void drawAll(Points& cores, std::vector<Points>& vec, ShaderProg&);
inline void k_means(Points& points, Points& cores, std::vector<Points>& vec, ShaderProg&, GLFWwindow*);
ImPlotPoint normalFunc(int idx, void* data);

struct normalData
{
    double x0;
    double h;
    double mathExpect;
    double deviation;
    float probability;
    double max;
};

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "K-means", NULL, NULL);
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

    Points class1(pointsNum);
    Points class2(pointsNum / 4);

#ifdef REAL_DATA
    std::fstream in;
    in.open("../../Mall_Customers.csv");

    size_t index = 0;
    while(!in.eof())
    {
        glm::vec2 temp;
        char trash;
        in >> temp.x >> trash >> temp.y;
        points[index++] = temp;
    }
#else
    class1.normalFill(4.0f, 1.0f);
    class1.setColor(colors[1]);

    class2.normalFill(7.0f, 0.3f);
    class2.setColor(colors[3]);
#endif
    
    glPointSize(4);
    glClearColor(0.f, 0.f, 0.f, 0.0f);
    Point2D point{glm::vec2(1.0f)};
    point.setColor(glm::vec3(1.0f));
    float probability1C = 0.5f, probability2C{};
    bool plot = false;
    while (!glfwWindowShouldClose(window))
    {
        processInput(window, point);
        Renderer::Clear();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui::ShowDemoWindow();
        
        auto pos = point.getPos();
        ImGui::Begin("Place a point");
        ImGui::SliderFloat2("Pos", glm::value_ptr(pos), minCoord, maxCoord);
        ImGui::SliderFloat("Probability assing to 1st class", &probability1C, 0.0f, 1.0f);
        probability2C = 1.0f - probability1C;
        ImGui::SliderFloat("Probability assing to 2nd class", &probability2C, 0.0f, 1.0f);
        probability1C = 1.0f - probability2C;
        plot ^= ImGui::Button("Calculate", ImVec2(100, 40));
        ImGui::End();
        point.setPos(pos);

        if(plot)
        {
            normalData data1{0.0, 0.01, 4.0, 1.0, probability1C, 0.0};
            normalData data2{0.0, 0.01, 7.0, 0.3, probability2C, 0.0};
            ImGui::Begin("Plot", &plot);

            ImPlot::BeginPlot("Plot");
            ImPlot::PlotLineG("Class 1", normalFunc, &data1, 1000);
            ImPlot::PlotLineG("Class 2", normalFunc, &data2, 1000);
            ImPlot::EndPlot();

            std::string classNumb = std::to_string(data1.max > data2.max ? 1 : 2);
            ImGui::Text("%f max probability for class %s", std::max(data1.max, data2.max), classNumb.c_str());

            ImGui::End();
        }


        glPointSize(4);
        Renderer::Draw(class1, base);
        Renderer::Draw(class2, base);
        glPointSize(6);
        Renderer::Draw(point, base);

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
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window, Point2D& point)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);


    auto speed = 0.01f;
    if(glfwGetKey(window,  GLFW_KEY_UP) == GLFW_PRESS)
        point.setPos(point.getPos() + glm::vec2(0.f, speed));
    if(glfwGetKey(window,  GLFW_KEY_DOWN) == GLFW_PRESS)
        point.setPos(point.getPos() + glm::vec2(0.f, -speed));
    if(glfwGetKey(window,  GLFW_KEY_RIGHT) == GLFW_PRESS)
        point.setPos(point.getPos() + glm::vec2(speed, 0.f));
    if(glfwGetKey(window,  GLFW_KEY_LEFT) == GLFW_PRESS)
        point.setPos(point.getPos() + glm::vec2(-speed, 0.f));
}

ImPlotPoint normalFunc(int idx, void *data)
{
    static constexpr double sqrt2pi = M_2_SQRTPI * M_SQRT1_2 / 2; // 1/sqrt(2pi)

    auto ndata = (normalData*)data;
    double x{ndata->x0 + idx * ndata->h}, y{};

    y = sqrt2pi / ndata->deviation * exp(-0.5f * pow((x - ndata->mathExpect) / ndata->deviation, 2)) * ndata->probability;
    ndata->max = std::max(ndata->max, y);
    return ImPlotPoint(x, y);
}

void distribute(const Points &points, const Points &cores, std::vector<Points> &vec)
{
    for(int j = 0; j < pointsNum; ++j)
    {
        auto& point = points[j];
        int minCoreInd = 0;
        for(int i = 1; i < coresNum; ++i)
        {
            if(glm::length(point - cores[minCoreInd]) > glm::length(point - cores[i]))
                minCoreInd = i;
        }
        vec[minCoreInd].Push(point);
    }
}

void setColors(std::vector<Points> &vec)
{
    auto iter = colors.begin();
    for(auto& points : vec)
    {
        points.setColor(*iter++);
    }
}

void drawAll(Points &cores, std::vector<Points>& vec, ShaderProg& base)
{
    Renderer::Clear();
    glPointSize(4);
    for(auto& points : vec)
    {
        Renderer::Draw(points, base);
    }
    glPointSize(5);
    Renderer::Draw(cores, base);
}

void k_means(Points &points, Points &cores, std::vector<Points> &vec, ShaderProg & base, GLFWwindow *window)
{
    bool changed = true;
    while(changed)
    {
        changed = false;
        for(int index = 0; index < vec.size(); ++index)
        {
            auto& points = vec[index];
            auto size = points.getSize();

            glm::vec2 sum = {0, 0};
            for(int i = 0; i < size; ++i)
            {
                auto& point = points[i];
                sum += point;
            }
            sum /= double(size);

            auto& core = cores[index];
            changed = glm::length(sum - core) > 0.00001;
            if(changed)
            {
                core.x = sum.x;
                core.y = sum.y;
                cores.fillBuffer();
            }
            points.clear();
        }

        distribute(points, cores, vec);

        drawAll(cores, vec, base);
        glfwSwapBuffers(window);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

