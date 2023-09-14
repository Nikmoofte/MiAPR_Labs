#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "ShaderProg/ShaderProg.h"
#include <VBO/VBO.h>
#include <VAO/VAO.h>
#include <VAO/VBLayout.h>
#include <EBO/EBO.h>
#include <Renderer/Renderer.h>
#include <Camera/Camera.h>


#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <Points/Points.hpp>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const char* glsl_version = "#version 330";

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

extern const float maxCoord = 10.f; 
extern const float minCoord = 0.f; 

unsigned coresNum = 6;
unsigned pointsNum = 1000;

std::vector<vec3> colors = {
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

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    ShaderProg base("../../shaders/Base.vs", "../../shaders/Base.fs");
    glUniform1f(base.GetLocation("maxCoord"), maxCoord);

    Points points(pointsNum);
    points.randomFill();
    
    Points cores(coresNum);
    cores.setColor({1.f, 1.f, 1.f});
    cores.randomFill();

    std::vector<Points> vec(coresNum);
    auto iter = colors.begin();
    for(auto& points : vec)
    {
        points.setColor(*iter++);
    }
    
    for(int j = 0; j < pointsNum; ++j)
    {
        auto& point = points[j];
        int minCoreInd = 0;
        for(int i = 1; i < coresNum; ++i)
        {
            if((point - cores[minCoreInd]).getDist() > (point - cores[i]).getDist())
                minCoreInd = i;
        }
        vec[minCoreInd].Push(point);
    }
    
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    auto prevPointsNum = pointsNum;
    auto prevCoresNum = coresNum;
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Settings");                          // Create a window called "Hello, world!" and append into it.
        ImGui::SliderInt("Num of points", (int*)&pointsNum, 1, 100000, "%u");
        ImGui::SliderInt("Num of cores", (int*)&coresNum, 2, 10, "%u");
        bool distribute = ImGui::Button("Distribute");

        ImGui::End();

        bool change = false;
        if(coresNum != prevCoresNum)
        {
            cores.resize(coresNum);
            cores.randomFill(prevCoresNum);

            prevCoresNum = coresNum;
            change = true;
        }

        if(pointsNum != prevPointsNum)
        {
            points.resize(pointsNum);
            points.randomFill(prevPointsNum);

            prevPointsNum = pointsNum;
            change = true;
        }

        if(change)
        {
            vec.clear();
            vec.resize(coresNum);
            auto iter = colors.begin();
            for(auto& points : vec)
            {
                points.setColor(*iter++);
            }
            for(int j = 0; j < pointsNum; ++j)
            {
                auto& point = points[j];
                int minCoreInd = 0;
                for(int i = 1; i < coresNum; ++i)
                {
                    if((point - cores[minCoreInd]).getDist() > (point - cores[i]).getDist())
                        minCoreInd = i;
                }
                vec[minCoreInd].Push(point);
            }

            change = false;
        }

        Renderer::Clear();
        glPointSize(4);
        for(auto& points : vec)
        {
            Renderer::Draw(points, base);
        }
        glPointSize(5);
        Renderer::Draw(cores, base);



        if(distribute)
        {
            bool changed = true;
            while(changed)
            {
                changed = false;
                for(int index = 0; index < vec.size(); ++index)
                {
                    auto& points = vec[index];
                    auto size = points.getSize();

                    vec2 sum = {0, 0};
                    for(int i = 0; i < size; ++i)
                    {
                        auto& point = points[i];
                        sum += point;
                    }
                    sum /= double(size);

                    auto& core = cores[index];
                    changed = (sum - core).getDist() > 0.001;
                    if(changed)
                    {
                        core.x = sum.x;
                        core.y = sum.y;
                        cores.fillBuffer();
                    }
                    points.clear();
                }

                for(int j = 0; j < pointsNum; ++j)
                {
                    auto& point = points[j];
                    int minCoreInd = 0;
                    for(int i = 1; i < coresNum; ++i)
                    {
                        if((point - cores[minCoreInd]).getDist() > (point - cores[i]).getDist())
                            minCoreInd = i;
                    }
                    vec[minCoreInd].Push(point);
                }

                Renderer::Clear();
                glPointSize(4);
                for(auto& points : vec)
                {
                    Renderer::Draw(points, base);
                }
                glPointSize(5);
                Renderer::Draw(cores, base);
                glfwSwapBuffers(window);

            }
        }

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


    glfwTerminate();
    return 0;
}


void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

