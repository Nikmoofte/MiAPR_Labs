cmake_minimum_required(VERSION 3.5)
project(WinOPGLProj)

set(ClassesDir src/classes/)
set(ImguiPath deps/imgui/)

set(Classes 
    ${ClassesDir}Camera/Camera.cpp
    ${ClassesDir}ShaderProg/ShaderProg.cpp
    ${ClassesDir}Renderer/Renderer.cpp
    ${ClassesDir}VBO/VBO.cpp
    ${ClassesDir}VAO/VAO.cpp
    ${ClassesDir}VAO/VBlayout.cpp
    ${ClassesDir}EBO/EBO.cpp

    ${ClassesDir}Points/Points.cpp

    ${ImguiPath}imgui_demo.cpp
    ${ImguiPath}imgui_draw.cpp
    ${ImguiPath}imgui_tables.cpp
    ${ImguiPath}imgui_widgets.cpp
    ${ImguiPath}imgui.cpp
    ${ImguiPath}backends/imgui_impl_opengl3.cpp
    ${ImguiPath}backends/imgui_impl_glfw.cpp
    )


add_executable(main src/main.cpp include/glad/glad.c src/stb_image.cpp ${Classes})

add_subdirectory(deps/glfw)
add_subdirectory(deps/glm)
target_link_libraries(main PUBLIC glfw)
target_link_libraries(main PUBLIC glm)
target_link_libraries(main PUBLIC opengl32)
target_include_directories(main PUBLIC include ${ClassesDir})
target_include_directories(main PUBLIC include ${ImguiPath})