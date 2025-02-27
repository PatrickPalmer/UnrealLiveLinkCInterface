cmake_minimum_required(VERSION 3.30)

project(UnrealLiveLinkCInterfaceAPI)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(${PROJECT_NAME} STATIC src/UnrealLiveLinkCInterfaceAPI.c)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    PUBLIC_HEADER "include/UnrealLiveLinkCInterfaceAPI.h;include/UnrealLiveLinkCInterfaceTypes.h"
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/include
)

