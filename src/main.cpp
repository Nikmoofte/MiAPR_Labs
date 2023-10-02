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


#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <Points/Points.hpp>

#define REAL_DATA

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

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

    Points points(pointsNum);

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
    points.randomFill();
#endif


    Points cores(coresNum);
    cores.setColor({1.f, 1.f, 1.f});
    cores.randomFill();

    std::vector<Points> vec(coresNum);
    setColors(vec);
    
    distribute(points, cores, vec);
    
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    auto prevPointsNum = pointsNum;
    auto prevCoresNum = coresNum;
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Settings", 0, ImGuiWindowFlags_NoResize); 
        ImGui::SetWindowSize({400, 200});
        ImGui::SliderInt("Num of points", (int*)&pointsNum, 1, 100000, "%u");
        ImGui::SliderInt("Num of cores", (int*)&coresNum, 2, 10, "%u");
        bool kmeans = ImGui::Button("K-means");
        bool minimax = ImGui::Button("minimax");

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
            setColors(vec);
            distribute(points, cores, vec);

            change = false;
        }

        drawAll(cores, vec, base);

        if(kmeans)
        {
            k_means(points, cores, vec, base, window);
        }

        if(minimax)
        {
            coresNum = 1;
            cores.resize(coresNum);
            cores.randomFill();

            prevCoresNum = coresNum;

            bool changed = true;
            while(changed)
            {
                vec.clear();
                vec.resize(coresNum);
                setColors(vec);
                distribute(points, cores, vec);
                k_means(points, cores, vec, base, window);

                changed = false;
                glm::vec2 maxDistPoint = vec[0][0];
                auto maxDist = glm::length(cores[0] - maxDistPoint);
                for(int i = 0; i < coresNum; ++i)
                {
                    auto& points = vec[i];
                    auto& core = cores[i];
                    for(int j = 0; j < points.getSize(); ++j)
                    {
                        auto& point = points[j];
                        if(maxDist < glm::length(core - point))
                        {
                            maxDistPoint = point;
                            maxDist = glm::length(core - maxDistPoint);
                        }
                    }
                }

                float sum = 0.0f;

                auto end = cores.getSize();
                for(size_t i = 0; i < end - 1; ++i)
                {
                    for (size_t j = i + 1; j < end; ++j)
                    {
                        sum += glm::length(cores[i] - cores[j]);
                    }   
                }
                sum /= (end * (end - 1));

                if(cores.getSize() <= 1 || maxDist > sum)
                {
                    cores.Push(maxDistPoint);
                    ++coresNum;
                    prevCoresNum = coresNum;
                    changed = true;
                }

                drawAll(cores, vec, base);
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

