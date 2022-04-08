cmake_minimum_required(VERSION 3.5)

project(hellogles3_cmake VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_subdirectory(shaderc EXCLUDE_FROM_ALL)

find_package(GLEW REQUIRED)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets OpenGL)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGL)


set(PROJECT_SOURCES
        main.cpp
        glwindow.cpp
        glwindow.h
        logo.cpp
        logo.h
        vertex_shader_1.vert
        fragment_shader_1.frag
        shaders/vertex_shader.vert
        shaders/fragment_shader.frag
        shaders/dsModel.glsl
        hellogles3.qrc
        shaders.qrc
)

find_program(glslc_executable NAMES glslc)

#function(compile_shader_cmake)
#    cmake_parse_arguments(PARSE_ARGV 0 arg "" "" "SHADERS")
#    foreach(shader ${arg_SHADERS})
#        execute_process(
#            COMMAND ${glslc_executable}
#            --target-env=opengl
#            -mfmt=bin
#            -fauto-map-locations
#            -fauto-bind-uniforms
#            -MD -MF ${CMAKE_CURRENT_SOURCE_DIR}/${shader}.d
#            -o ${CMAKE_CURRENT_SOURCE_DIR}/${shader}.spv
#            ${CMAKE_CURRENT_SOURCE_DIR}/${shader}
#        )
#    endforeach()
#endfunction()

#compile_shader_cmake(
#    SHADERS
#        shaders/vertex_shader.vert
#        shaders/fragment_shader.frag
#)

function(compile_shader_make target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "ENV;FORMAT" "SHADERS")
    foreach(shader ${arg_SHADERS})
        add_custom_command(
            PRE_BUILD
            OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${shader}.spv
            DEPENDS ${shader}
            DEPFILE ${shader}.d
            COMMAND
                ${glslc_executable}
                $<$<BOOL:${arg_ENV}>:--target-env=${arg_ENV}>
                $<$<BOOL:${arg_FORMAT}>:-mfmt=${arg_FORMAT}>
                -fauto-map-locations
                -fauto-bind-uniforms
                -MD -MF ${CMAKE_CURRENT_SOURCE_DIR}/${shader}.d
                -o ${CMAKE_CURRENT_SOURCE_DIR}/${shader}.spv
                ${CMAKE_CURRENT_SOURCE_DIR}/${shader}
        )
        #target_sources(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${shader}.spv)
    endforeach()
endfunction()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(hellogles3_cmake
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET hellogles3_cmake APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(hellogles3_cmake SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(hellogles3_cmake
            ${PROJECT_SOURCES}
        )
    endif()
endif()

compile_shader_make(hellogles3_cmake
    ENV opengl
    FORMAT bin
    SHADERS
        shaders/vertex_shader.vert
        shaders/fragment_shader.frag
)

target_link_libraries(hellogles3_cmake PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::OpenGL PUBLIC GLEW::GLEW)

set_target_properties(hellogles3_cmake PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(hellogles3_cmake)
endif()

