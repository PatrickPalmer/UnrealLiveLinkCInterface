cmake_minimum_required(VERSION 3.13)

project(UnrealLiveLinkCInterfaceAPI VERSION 1.2.0 DESCRIPTION "Unreal Live Link C Interface")

add_library(${PROJECT_NAME} STATIC src/UnrealLiveLinkCInterfaceAPI.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER "include/UnrealLiveLinkCInterfaceAPI.h;include/UnrealLiveLinkCInterfaceTypes.h")

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/include
)

