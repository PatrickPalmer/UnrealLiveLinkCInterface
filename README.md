# Unreal Live Link C interface
Version 2.0.0

For Unreal Engine v5.4 or greater.

## Overview

This small library provides a C Interface to the Unreal Live Link Message Bus API. This allows for third party packages to stream to Unreal Live Link without requiring to compile with the Unreal Build Tool (UBT). This is done by exposing the Live Link API via a C interface in a shared object compiled using UBT. This shared object is loaded via an API and exposed functions can be called with a light weight interface.

The Unreal Live Link Message Bus API is fully exposed except the per frame metadata scene time is limited to just SMPTE timecode.  

## Building

### Building the Unreal DLL

Building the Unreal Live Link C Interface plugin DLL requires the Unreal Engine source code. Download the source code from [github](https://github.com/EpicGames/UnrealEngine).

#### Windows

 * copy the UnrealLiveLinkCInterface directory from this repository to [UNREAL_ENGINE_SRC_LOCATION]\Engine\Source\Programs
 * copy the file include/UnrealLiveLinkCInterfaceTypes.h from this repository to [UNREAL_ENGINE_SRC_LOCATION]\Engine\Source\Programs\UnrealLiveLinkCInterface directory
 * cd [UNREAL_ENGINE_SRC_LOCATION]
 * run "GenerateProjectFiles.bat", this creates UE5.sln Visual Studio Solution File
 * load UE5.sln in Visual Studio 2022
 * build
 * the DLL can be found at [UNREAL_ENGINE_SRC_LOCATION]\Engine\Binaries\Win64\UnrealLiveLinkCInterface.dll

### Building the library

Building on Windows
```
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
```

Cmake Options
* BUILD_EXAMPLES (default OFF) - build C example using the library
* BUILD_PYTHON_MODULE (default ON) - build python module
 
## Link C Interface library to specific or multiple instances of Unreal

By default, C Interface library will link to a single instance of Unreal running on the same device. However, if you are on a local area network where multiple devices are being used, you can link to specific instances of Unreal using a Unicast Endpoint and Static Endpoint to act as each end of the bridge.

To set up these network endpoints, you will first need the IPv4 address of the device running C Interface library and the device running Unreal.

To find your IPv4 address:
* On Windows: Open a command prompt and run an ipconfig command.
* on Linux: Use the command ifconfig -a.

Once you have your IPv4 values, you can then proceed to set up the link depending on the two scenarios outlined below:

* In C Interface library, call the appropriate C functions to set the endpoints.
* In Unreal, open the Project Settings and go to the UDP Messaging section.
* Enter the IPv4 value into C Interface Library and Unreal based on the following scenarios:

  * If you want C Interface Library to keep track the various instances of Unreal that it should connect to:
    1. Input the target IPv4 into Unreal's Unicast Endpoint field.
    2. Add that same value to the list of C Interface Library's Static Endpoints function.
    3. Repeat this for each device running Unreal.
  * If you want each instance of Unreal to keep track of the instance of C Interface Library it should connect to:
    1. Input the IPv4 value into C Interface Library's Unicast Endpoint function.
    2. Add the same value to Unreal's Multicast Endpoint field.
    3. Repeat step 2 for each instance of Unreal.

## Design considerations

I wanted to use C language (C89) for the API as it has the smallest requirements to interface with any language. ANSI standard C89 was choosen because it is compatible with Microsoft Visual Studio. Visual Studio 2019 partially support C99/C11 so at some point, when eventually cutting over to supporting just Unreal Engine v5, will move the code base forward.

Boolean types between C and C++ standards are not 100% compatible so I chose to avoid using bool type. I'm on the fence about creating a custom bool type so the function signatures and structures provide explicit declarations but for now, booleans are passed as ints and contain values 0 and 1. The code treats boolean types as 0 and not 0 for all tests.  

This middleware code adds an extra layer between the third party software the Unreal Live Link. In doing so, it adds at least 1 additional memory copy for all data (both the initialization and the per frame data). In my case of a few thousand float values updating at 60Hz wasn't a concern. The Python module also adds an additional memory copies.

The Motion Builder Unreal Live Link DLL provided much inspiration.

## Examples

To try the Circling Transform example, build with the example cmake option turned on (BUILD_EXAMPLES=ON). Within Unreal, add the Live Link plugin to the current project and restart. Run the Circling Tranform example program and while it is running, add the CirclingTransform Provider in the Unreal Live Link Manager Window. Create a cube and add a Live Link component with the subject set to "CirclingTransform". The cube should be circling in the viewport.

There is also a few Python examples in the pyUnrealLiveLink/examples directory.

## Future work

 * linux testing
 * OSX support

## Links

 * [Visual Studio 2019: C11 and C17 Standard Support Arriving in MSVC](https://devblogs.microsoft.com/cppblog/c11-and-c17-standard-support-arriving-in-msvc/)
 * [Motion Builder Live Link github](https://github.com/ue4plugins/MobuLiveLink)

