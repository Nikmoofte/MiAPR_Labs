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
double delta = 0.0000001;

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

struct normalData
{
    double x0;
    double h;
    double mathExpect;
    double deviation;
    float probability;
    double max;
};

inline void setColors(std::vector<Points>& vec);
inline void drawAll(Points& cores, std::vector<Points>& vec, ShaderProg&);
inline double normalFunc(double x, double mathExpect, double deviation);
ImPlotPoint normalFunc(int idx, void* data);
double simpson_rule(double a, double b,
                    int n, // Number of intervals
                    double mathExpect, double deviation)
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
glm::vec2 intersection(const normalData& data1, const normalData& data2, double delta);


int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Statistic distribution", NULL, NULL);
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
    float probability1C{0.5f}, probability2C{0.5f};
    normalData data1{0.0, 0.01, 4.0, 1.0, probability1C, 0.0};
    normalData data2{0.0, 0.01, 7.0, 1.0, probability2C, 0.0};

    class1.normalFill(data1.mathExpect, data1.deviation);
    class1.setColor(colors[1]);
    data1.mathExpect = class1.getMathExpectence();
    data1.deviation = class1.getDeviation(data1.mathExpect);

    class2.normalFill(data2.mathExpect, data2.deviation);
    class2.setColor(colors[2]);
    data2.mathExpect = class2.getMathExpectence();
    data2.deviation = class2.getDeviation(data2.mathExpect);

    auto intr = intersection(data1, data2, delta);
#endif
    
    glPointSize(4);
    glClearColor(0.f, 0.f, 0.f, 0.0f);
    Point2D point{glm::vec2(1.0f)};
    point.setColor(glm::vec3(1.0f));

    bool calcProbability = false;
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
        calcProbability = ImGui::Button("Calculate", ImVec2(100, 40));
        ImGui::End();
        point.setPos(pos);
        if(abs(data1.probability - probability1C) > delta)
        {
            data1.probability = probability1C;
            data2.probability = probability2C;
            intr = intersection(data1, data2, delta);
        }
        if(calcProbability)
        {
            auto& Class1list = class1.getList();
            for(auto beg = Class1list.begin(), end = Class1list.end(); beg != end;)
            {
                data1.max = normalFunc(beg->x, data1.mathExpect, data1.deviation) * probability1C;
                data2.max = normalFunc(beg->x, data2.mathExpect, data2.deviation) * probability2C;
                if(data1.max < data2.max)
                {
                    class2.Push(*beg);
                    beg = Class1list.erase(beg);
                }
                else
                    ++beg;
            }
            class1.fillBuffer();

            auto& Class2list = class2.getList();
            for(auto beg = Class2list.begin(), end = Class2list.end(); beg != end; )
            {
                data1.max = normalFunc(beg->x, data1.mathExpect, data1.deviation) * probability1C;
                data2.max = normalFunc(beg->x, data2.mathExpect, data2.deviation) * probability2C;
                if(data1.max > data2.max)
                {
                    class1.Push(*beg);
                    beg = Class2list.erase(beg);
                }
                else
                    ++beg;
            }
            class2.fillBuffer();
        }

        static double difPercent1C{}, difPercent2C{};
        static double difDevPercent1C{}, difDevPercent2C{};
         
        /*if(calcProbability)
        {
            auto mathExpect1C = class1.getMathExpectence() * class1.getSize();
            auto mathExpect2C = class2.getMathExpectence() * class2.getSize();
            
            difPercent1C = (mathExpect1C + glm::length(pos)) / mathExpect1C - 1;
            difPercent2C = (mathExpect2C + glm::length(pos)) / mathExpect2C - 1;

            mathExpect1C = (mathExpect1C + glm::length(pos)) / class1.getSize();
            mathExpect2C = (mathExpect2C + glm::length(pos)) / class2.getSize();

            double dev1C = class1.getDeviation(mathExpect1C) * class1.getSize();
            double dev2C = class1.getDeviation(mathExpect2C) * class2.getSize();

            difDevPercent1C = (dev1C + pow(glm::length(pos) - mathExpect1C, 2)) / dev1C - 1;
            difDevPercent2C = (dev2C + pow(glm::length(pos) - mathExpect2C, 2)) / dev2C - 1;

            probability1C = (1 / ((difDevPercent1C + difPercent1C) / (difDevPercent2C + difPercent2C)));
        }*/
        data1.max = normalFunc(pos.x, data1.mathExpect, data1.deviation) * probability1C;
        data2.max = normalFunc(pos.x, data2.mathExpect, data2.deviation) * probability2C;

        //ImPlot::ShowDemoWindow();

        ImGui::Begin("Plot");

        ImPlot::BeginPlot("Plot");
        ImPlot::PlotLineG("Class 1", normalFunc, &data1, 1000);
        ImPlot::PlotLineG("Class 2", normalFunc, &data2, 1000);
        auto fillData = data2;
        fillData.x0 = intr.x - 2.0f;
        ImPlot::PlotShadedG("Zone 1", normalFunc, &fillData, [](int idx, void *data){ auto ndata = (normalData*)data; return ImPlotPoint(ndata->x0 + idx * ndata->h, 0);}, &fillData, 201);
        fillData = data1;
        fillData.x0 = intr.x;
        ImPlot::PlotShadedG("Zone 2", normalFunc, &fillData, [](int idx, void *data){ auto ndata = (normalData*)data; return ImPlotPoint(ndata->x0 + idx * ndata->h, 0);}, &fillData, 200);
        double xp[]{pos.x, pos.x}, yp[]{0.0, 0.5};
        ImPlot::PlotLine("Pos", xp, yp, 2);
        ImPlot::EndPlot();
        int classNumb = data1.max > data2.max ? 1 : 2;
        point.setColor(colors[classNumb]);
        ImGui::Text("max probability = %f for class %s", std::max(data1.max, data2.max), std::to_string(classNumb).c_str());

        auto LTerror = simpson_rule(intr.x - 2.0, intr.x, 1000, data2.mathExpect, data2.deviation);
        auto POerror = simpson_rule(intr.x, intr.x + 2.0, 1000, data1.mathExpect, data1.deviation);
        ImGui::Text("False alarm: %f", LTerror);
        ImGui::Text("Detection skip: %f", POerror);
        ImGui::Text("Error sum: %f", POerror + LTerror);

        /*if(calcProbability)
        {
            ImGui::Text("dif 1 class %e", difPercent1C);
            ImGui::Text("dif 2 class %e", difPercent2C);

            ImGui::Text("\ndev dif 1 class %e", difDevPercent1C);
            ImGui::Text("dev dif 2 class %e", difDevPercent2C);
        }*/

        ImGui::End();

        glPointSize(4);
        Renderer::Draw(class1, base);
        Renderer::Draw(class2, base);
        glPointSize(10);
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


    auto speed = 0.02f;
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
    auto ndata = (normalData*)data;
    double x{ndata->x0 + idx * ndata->h}, y{};

    y = normalFunc(x, ndata->mathExpect, ndata->deviation) * ndata->probability;
    return ImPlotPoint(x, y);
}

inline double normalFunc(double x, double mathExpect, double deviation)
{
    static constexpr double sqrt2pi = M_2_SQRTPI * M_SQRT1_2 / 2; // 1/sqrt(2pi)
    return sqrt2pi / deviation * exp(-0.5f * pow((x - mathExpect) / deviation, 2));
}

glm::vec2 intersection(const normalData& data1, const normalData& data2, double delta)
{
    double step = 0.001;
    for(double min = 2.0; min < maxCoord; min += step)
    {
        if(normalFunc(min, data1.mathExpect, data1.deviation) * data1.probability - normalFunc(min, data2.mathExpect, data2.deviation) * data2.probability < delta)
            return glm::vec2(min, normalFunc(min, data1.mathExpect, data1.deviation) * data1.probability);
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}