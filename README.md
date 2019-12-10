# Unreal Live Link C interface
Version 1.3.0

For Unreal v4.23 or greater.

## Overview

This small library provides a C Interface to the Unreal Live Link Message Bus API.   This allows for third party packages to stream to Unreal Live Link without requiring to compile with the Unreal Build Tool (UBT).   This is done by exposing the Live Link API via a C interface in a shared object compiled using UBT.    This shared object is loaded via an API and exposed functions can be called with a light weight interface.

The Unreal Live Link Message Bus API is fully exposed except the per frame metadata scene time is limited to just SMPTE timecode.  

## Building the Unreal DLL

Building the Unreal Live Link C Interface plugin DLL requires the Unreal Engine source code.   Download the source code from [github](https://github.com/EpicGames/UnrealEngine).

### Windows

 * copy the UnrealLiveLinkCInterface directory from this repository to [UNREAL_ENGINE_SRC_LOCATION]\Engine\Source\Programs
 * copy the file include/UnrealLiveLinkCInterfaceTypes.h from this repository to [UNREAL_ENGINE_SRC_LOCATION]\Engine\Source\Programs\UnrealLiveLinkCInterface directory
 * cd [UNREAL_ENGINE_SRC_LOCATION]
 * run "GenerateProjectFiles.bat", this creates UE4.sln Visual Studio Solution File
 * load UE4.sln in Visual Studio 2017
 * build
 * the DLL can be found at [UNREAL_ENGINE_SRC_LOCATION]\Engine\Binaries\Win64\UnrealLiveLinkCInterface.dll

## Design considerations

I wanted to use C language (C89) for the API as it has the smallest requirements to interface with any language.   ANSI standard C89 was choosen because it is compatible with Microsoft Visual Studio.

This middleware code adds an extra layer between the third party software the Unreal Live Link.   In doing so, it adds at least 1 additional memory copy for all data (both the initialization and the per frame data).   In my case of a few thousand float values updating at 60Hz wasn't a concern.

The Maya Unreal Live Link DLL provided much inspiration.

## Future work

 * examples
 * linux support

