cmake_minimum_required(VERSION 3.0)
project(opengl-imgui-sample CXX)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

include_directories(
        /home/vitalii/.conan/data/imgui/1.74/_/_/package/d351525cc53ebe68279edf1978846402420066e7/include/
        /home/vitalii/.conan/data/glfw/3.3.2/_/_/package/4b30c908451427cc9b87e66ae92d9ceb616553c2/include/
        /home/vitalii/.conan/data/glew/2.1.0/_/_/package/2236b2cb703ee4f00965194fb773deafecabac53/include/
        /home/vitalii/.conan/data/fmt/7.0.3/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include/
        /home/vitalii/.conan/data/glm/0.9.9.8/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include/
)

# CONFIG option is important so that CMake doesnt search for modules into the default modules directory
find_package(imgui REQUIRED CONFIG)
find_package(glfw REQUIRED CONFIG)
find_package(glew REQUIRED CONFIG)
find_package(fmt REQUIRED CONFIG)
find_package(glm REQUIRED CONFIG)

add_executable( opengl-imgui-sample
                main.cpp
                model.cpp
                model.h
                opengl_shader.cpp
                opengl_shader.h
                3rd-party/stb_image.h
                3rd-party/stb_image.cpp
                3rd-party/tiny_obj_loader.cpp
                3rd-party/opengl.h
                bindings/imgui_impl_glfw.cpp
                bindings/imgui_impl_opengl3.cpp
                bindings/imgui_impl_glfw.h
                bindings/imgui_impl_opengl3.h
        assets/model_shader.vs
        assets/model_shader.fs
        assets/cubemap_shader.vs
        assets/cubemap_shader.fs
        assets/simple_shader.vs
        assets/simple_shader.fs
        assets/water_shader.vs
        assets/water_shader.fs
        assets/landscape_shader.vs
        assets/landscape_shader.fs
        assets/empty_shader.fs)

add_custom_command(TARGET opengl-imgui-sample
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/model_shader.vs ${PROJECT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/model_shader.fs ${PROJECT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/cubemap_shader.vs ${PROJECT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/cubemap_shader.fs ${PROJECT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/simple_shader.vs ${PROJECT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/simple_shader.fs ${PROJECT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/water_shader.vs ${PROJECT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/water_shader.fs ${PROJECT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/landscape_shader.vs ${PROJECT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/landscape_shader.fs ${PROJECT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/empty_shader.fs ${PROJECT_BINARY_DIR}
        )

target_compile_definitions(opengl-imgui-sample PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_link_libraries(opengl-imgui-sample imgui::imgui GLEW::glew_s glfw::glfw fmt::fmt glm::glm)
