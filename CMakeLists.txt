cmake_minimum_required(VERSION 3.13)

project(UnrealLiveLinkCInterfaceAPI VERSION 1.4.0 DESCRIPTION "Unreal Live Link C Interface")

OPTION(BUILD_EXAMPLES "Build examples" OFF)

add_library(${PROJECT_NAME} STATIC src/UnrealLiveLinkCInterfaceAPI.c)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER "include/UnrealLiveLinkCInterfaceAPI.h;include/UnrealLiveLinkCInterfaceTypes.h")

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

if (BUILD_EXAMPLES)
    ADD_SUBDIRECTORY(examples)
endif(BUILD_EXAMPLES)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/include
)

