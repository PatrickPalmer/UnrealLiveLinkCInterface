cmake_minimum_required(VERSION 3.30)

project(UnrealLiveLinkCInterface VERSION 2.0.0 DESCRIPTION "Unreal Live Link C Interface")

OPTION(BUILD_EXAMPLES "Build examples" OFF)
OPTION(BUILD_PYTHON_MODULE "Build Python Module" ON)

ADD_SUBDIRECTORY(src)

if (BUILD_PYTHON_MODULE)
    ADD_SUBDIRECTORY(pyUnrealLiveLink)
endif(BUILD_PYTHON_MODULE)

if (BUILD_EXAMPLES)
    ADD_SUBDIRECTORY(examples)
endif(BUILD_EXAMPLES)


