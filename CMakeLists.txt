cmake_minimum_required (VERSION 3.13)

project ("mvImporter")

add_definitions(-D_USE_MATH_DEFINES)

if(WIN32)
    add_subdirectory("Sandbox_D3D11")
elseif(APPLE)
    add_subdirectory("Sandbox_Metal")
endif()