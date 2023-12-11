cmake_minimum_required(VERSION 3.5)
project(Proj)

set(ClassesDir src/classes/)
set(ImguiPath deps/imgui/)
set(ImplotPath deps/implot/)

set(Classes 
    ${ClassesDir}ShaderProg/ShaderProg.cpp

    ${ImguiPath}imgui_demo.cpp
    ${ImguiPath}imgui_draw.cpp
    ${ImguiPath}imgui_tables.cpp
    ${ImguiPath}imgui_widgets.cpp
    ${ImguiPath}imgui.cpp
    ${ImguiPath}backends/imgui_impl_opengl3.cpp
    ${ImguiPath}backends/imgui_impl_glfw.cpp

    ${ImplotPath}implot_items.cpp
    ${ImplotPath}implot.cpp
    ${ImplotPath}implot_demo.cpp
)
add_executable(main src/main.cpp include/glad/glad.c include/stb_image/stb_image.cpp ${Classes})
set_property(TARGET main PROPERTY CXX_STANDARD 26)

add_subdirectory(deps/glfw)
add_subdirectory(deps/glm)
target_link_libraries(main PUBLIC glfw)
target_link_libraries(main PUBLIC glm)
target_link_libraries(main PUBLIC opengl32)
target_include_directories(main PUBLIC include)
target_include_directories(main PUBLIC ${ClassesDir})
target_include_directories(main PUBLIC ${ImguiPath})
target_include_directories(main PUBLIC ${ImplotPath})