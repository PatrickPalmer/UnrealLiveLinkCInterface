/** 
 * Copyright (c) 2025 Patrick Palmer
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include "UnrealLiveLinkCInterfaceAPI.h"
#include <string>
#include <array>
#include <vector>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

PYBIND11_MAKE_OPAQUE(std::vector<std::string>);
PYBIND11_MAKE_OPAQUE(std::vector<float>);


struct Transform
{
    std::array<float, 4> rotation = { 0, 0, 0, 0 };
    std::array<float, 3> translation = { 0, 0, 0 };
    std::array<float, 3> scale = { 1, 1, 1 };
};

typedef std::vector<std::string> Properties;
typedef std::vector<float> PropertyValues;

struct KeyValue
{
    std::string key;
    std::string value;
};

struct Metadata
{
    std::vector<KeyValue> keyValues;
    UnrealLiveLink_Timecode timecode;
};

struct Bone
{
    std::string name;
    int parentIndex{ -1 };      // -1 is root
};

PYBIND11_MAKE_OPAQUE(std::vector<Bone>);
PYBIND11_MAKE_OPAQUE(std::vector<Transform>);

typedef std::vector<Bone> AnimationStatic;
typedef std::vector<Transform> Animation;

struct Camera
{
    Transform transform;
    float fieldOfView{ 90.0f };
    float aspectRatio{ 1.777778f };
    float focalLength{ 50.0f };
    float aperture{ 2.8f };
    float focusDistance{ 100000.0f };
    int isPerspective{1};
};

struct Light
{
    Transform transform;
    float temperature{ 6500.0f };
    float intensity{ 3.1415926535897932f };
    std::array<uint8_t, 3> lightColor = { 255, 255, 255 };
    float innerConeAngle{ 0 };
    float outerConeAngle{ 44.0f };
    float attenuationRadius{ 1000.0f };
    float sourceRadius{ 0 };
    float softSourceRadius{ 0 };
    float sourceLength{ 0 };
};

static void CopyMetadata(const Metadata& metadata, UnrealLiveLink_Metadata& uellmeta)
{
    // TODO
    uellmeta.timecode = metadata.timecode;
}

typedef std::vector<std::array<char, UNREAL_LIVE_LINK_MAX_NAME_LENGTH>> NameCache;

static void CopyProperties(const Properties &properties, UnrealLiveLink_Properties &uellprop, NameCache &cache)
{
    uellprop.nameCount = properties.size();
    cache.reserve(properties.size());
    for (size_t i = 0; i < uellprop.nameCount; i++)
    {
        ::strncpy(cache[i].data(), properties[i].c_str(), UNREAL_LIVE_LINK_MAX_NAME_LENGTH);
    }
    uellprop.names = reinterpret_cast<UnrealLiveLink_Name*>(cache.data());
}

static void CopyPropertyValues(const PropertyValues& property_values, UnrealLiveLink_PropertyValues& uellpropval)
{
    uellpropval.values = const_cast<float*>(property_values.data());
    uellpropval.valueCount = property_values.size();
}

static void CopyTransform(const Transform& transform, UnrealLiveLink_Transform& uellTransform)
{
    for (size_t i = 0; i < 4; i++)
    {
        uellTransform.rotation[i] = transform.rotation[i];
    }
    for (size_t i = 0; i < 3; i++)
    {
        uellTransform.translation[i] = transform.translation[i];
        uellTransform.scale[i] = transform.scale[i];
    }
}

static void SetBasicStructure(const std::string& subject_name, const Properties& properties)
{
    if (UnrealLiveLink_SetBasicStructure != NULL) 
    {
        UnrealLiveLink_Properties uellprop;
        NameCache cache;
        CopyProperties(properties, uellprop, cache);

        UnrealLiveLink_SetBasicStructure(subject_name.c_str(), &uellprop);
    }
}

static void UpdateBasicFrame(const std::string & subject_name, const double world_time,
    const Metadata& metadata, const PropertyValues& property_values)
{
    if (UnrealLiveLink_UpdateBasicFrame != NULL)
    {
        UnrealLiveLink_Metadata uellmeta;
        CopyMetadata(metadata, uellmeta);

        UnrealLiveLink_PropertyValues uellpropval;
        CopyPropertyValues(property_values, uellpropval);

        UnrealLiveLink_UpdateBasicFrame(subject_name.c_str(), world_time, &uellmeta, &uellpropval);
    }
}

static void SetTransformStructure(const std::string& subject_name, const Properties& properties)
{
    if (UnrealLiveLink_SetTransformStructure != NULL) 
    {
        UnrealLiveLink_Properties uellprop;
        NameCache cache;
        CopyProperties(properties, uellprop, cache);

        UnrealLiveLink_SetTransformStructure(subject_name.c_str(), &uellprop);
    }
}

static void UpdateTransformFrame(const std::string & subject_name, const double world_time,
    const Metadata& metadata, const PropertyValues& property_values, const Transform &frame)
{
    if (UnrealLiveLink_UpdateTransformFrame != NULL)
    {
        UnrealLiveLink_Metadata uellmeta;
        CopyMetadata(metadata, uellmeta);

        UnrealLiveLink_PropertyValues uellpropval;
        CopyPropertyValues(property_values, uellpropval);

        UnrealLiveLink_Transform uelltransform;
        CopyTransform(frame, uelltransform);

        UnrealLiveLink_UpdateTransformFrame(subject_name.c_str(), world_time, &uellmeta, &uellpropval, &uelltransform);
    }
}

static void SetCameraStructure(const std::string& subject_name, const Properties& properties, UnrealLiveLink_CameraStatic &camera)
{
    if (UnrealLiveLink_SetCameraStructure != NULL) 
    {
        UnrealLiveLink_Properties uellprop;
        NameCache cache;
        CopyProperties(properties, uellprop, cache);

        UnrealLiveLink_SetCameraStructure(subject_name.c_str(), &uellprop, &camera);
    }
}

static void UpdateCameraFrame(const std::string & subject_name, const double world_time,
    const Metadata& metadata, const PropertyValues& property_values, Camera &camera)
{
    if (UnrealLiveLink_UpdateCameraFrame != NULL)
    {
        UnrealLiveLink_Metadata uellmeta;
        CopyMetadata(metadata, uellmeta);

        UnrealLiveLink_PropertyValues uellpropval;
        CopyPropertyValues(property_values, uellpropval);

        UnrealLiveLink_Camera uellcamera;
        CopyTransform(camera.transform, uellcamera.transform);
        uellcamera.fieldOfView = camera.fieldOfView;
        uellcamera.aspectRatio = camera.aspectRatio;
        uellcamera.focalLength = camera.focalLength;
        uellcamera.aperture = camera.aperture;
        uellcamera.focusDistance = camera.focusDistance;
        uellcamera.isPerspective = camera.isPerspective;

        UnrealLiveLink_UpdateCameraFrame(subject_name.c_str(), world_time, &uellmeta, &uellpropval, &uellcamera);
    }
}

static void SetLightStructure(const std::string& subject_name, const Properties& properties, UnrealLiveLink_LightStatic &light)
{
    if (UnrealLiveLink_SetLightStructure != NULL) 
    {
        UnrealLiveLink_Properties uellprop;
        NameCache cache;
        CopyProperties(properties, uellprop, cache);

        UnrealLiveLink_SetLightStructure(subject_name.c_str(), &uellprop, &light);
    }
}

static void UpdateLightFrame(const std::string & subject_name, const double world_time,
    const Metadata& metadata, const PropertyValues& property_values, Light &light)
{
    if (UnrealLiveLink_UpdateBasicFrame != NULL)
    {
        UnrealLiveLink_Metadata uellmeta;
        CopyMetadata(metadata, uellmeta);

        UnrealLiveLink_PropertyValues uellpropval;
        CopyPropertyValues(property_values, uellpropval);

        UnrealLiveLink_Light uelllight;
        CopyTransform(light.transform, uelllight.transform);
        uelllight.temperature = light.temperature;
        uelllight.intensity = light.intensity;
        for (size_t i = 0; i < 3; i++)
        {
            uelllight.lightColor[i] = light.lightColor[i];
        }
        std::array<uint8_t, 3> lightColor = { 255, 255, 255 };
        uelllight.innerConeAngle = light.innerConeAngle;
        uelllight.outerConeAngle = light.outerConeAngle;
        uelllight.attenuationRadius = light.attenuationRadius;
        uelllight.sourceRadius = light.sourceRadius;
        uelllight.softSourceRadius = light.softSourceRadius;
        uelllight.sourceLength = light.sourceLength;

        UnrealLiveLink_UpdateLightFrame(subject_name.c_str(), world_time, &uellmeta, &uellpropval, &uelllight);
    }
}

static void SetAnimationStructure(const std::string& subject_name, const Properties& properties, AnimationStatic &animation)
{
    if (UnrealLiveLink_SetAnimationStructure != NULL) 
    {
        UnrealLiveLink_Properties uellprop;
        NameCache cache;
        CopyProperties(properties, uellprop, cache);

        std::vector<UnrealLiveLink_Bone> bone_cache(animation.size());
        for (size_t i = 0; i < animation.size(); i++)
        {
            ::strncpy(bone_cache[i].name, animation[i].name.c_str(), UNREAL_LIVE_LINK_MAX_NAME_LENGTH);
            bone_cache[i].parentIndex = animation[i].parentIndex;
        }
        UnrealLiveLink_AnimationStatic uellanim;
        uellanim.bones = bone_cache.data();
        uellanim.boneCount = animation.size();

        UnrealLiveLink_SetAnimationStructure(subject_name.c_str(), &uellprop, &uellanim);
    }
}

static void UpdateAnimationFrame(const std::string & subject_name, const double world_time,
    const Metadata& metadata, const PropertyValues& property_values, Animation &animation)
{
    if (UnrealLiveLink_UpdateAnimationFrame != NULL)
    {
        UnrealLiveLink_Metadata uellmeta;
        CopyMetadata(metadata, uellmeta);

        UnrealLiveLink_PropertyValues uellpropval;
        CopyPropertyValues(property_values, uellpropval);

        UnrealLiveLink_Animation uellanim;
        uellanim.transforms = reinterpret_cast<UnrealLiveLink_Transform *>(animation.data());
        uellanim.transformCount = animation.size();

        UnrealLiveLink_UpdateAnimationFrame(subject_name.c_str(), world_time, &uellmeta, &uellpropval, &uellanim);
    }
}

PYBIND11_MODULE(pyUnrealLiveLink, m) {

    pybind11::enum_<UnrealLiveLink_TimecodeFormat>(m, "TimecodeFormat")
        .value("UNKNOWN", UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_UNKNOWN)
        .value("FC_23_98", UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_23_98)
        .value("FC_24", UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_24)
        .value("FC_25", UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_25)
        .value("FC_29_97_NDF", UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_29_97_NDF)
        .value("FC_29_97_DF", UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_29_97_DF)
        .value("FC_30", UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_30)
        .value("FC_47_95", UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_47_95)
        .value("FC_48", UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_48)
        .value("FC_50", UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_50)
        .value("FC_59_94_NDF", UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_59_94_NDF)
        .value("FC_59_94_DF", UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_59_94_DF)
        .value("FC_60", UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_60)
        .value("FC_72", UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_72)
        .value("FC_96", UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_96)
        .value("FC_100", UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_100)
        .value("FC_120", UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_120);

    pybind11::class_<UnrealLiveLink_Timecode>(m, "Timecode")
        .def(pybind11::init<>([]() {
            auto tc = UnrealLiveLink_Timecode();
            tc.hours = tc.minutes = tc.seconds = tc.frames = 0;
            tc.format = UnrealLiveLink_TimecodeFormat::UNREAL_LIVE_LINK_TIMECODE_UNKNOWN;
            return tc;
        }))
        .def_readwrite("hours", &UnrealLiveLink_Timecode::hours)
        .def_readwrite("minutes", &UnrealLiveLink_Timecode::minutes)
        .def_readwrite("seconds", &UnrealLiveLink_Timecode::seconds)
        .def_readwrite("frames", &UnrealLiveLink_Timecode::frames)
        .def_readwrite("format", &UnrealLiveLink_Timecode::format);

    pybind11::class_<UnrealLiveLink_CameraStatic>(m, "CameraStatic")
        .def(pybind11::init<>([]() { 
            auto cs = UnrealLiveLink_CameraStatic(); 
            UnrealLiveLink_InitCameraStatic(&cs); 
            return cs;
        }))
        .def_readwrite("is_field_of_view_supported", &UnrealLiveLink_CameraStatic::isFieldOfViewSupported)
        .def_readwrite("is_aspect_ratio_supported", &UnrealLiveLink_CameraStatic::isAspectRatioSupported)
        .def_readwrite("is_focal_length_supported", &UnrealLiveLink_CameraStatic::isFocalLengthSupported)
        .def_readwrite("is_projection_mode_supported", &UnrealLiveLink_CameraStatic::isProjectionModeSupported)
        .def_readwrite("film_back_width", &UnrealLiveLink_CameraStatic::filmBackWidth)
        .def_readwrite("film_back_height", &UnrealLiveLink_CameraStatic::filmBackHeight)
        .def_readwrite("is_aperture_supported", &UnrealLiveLink_CameraStatic::isApertureSupported)
        .def_readwrite("is_focus_distance_supported", &UnrealLiveLink_CameraStatic::isFocusDistanceSupported);

    pybind11::class_<Camera>(m, "Camera")
        .def(pybind11::init<>())
        .def_readwrite("transform", &Camera::transform)
        .def_readwrite("field_of_view", &Camera::fieldOfView)
        .def_readwrite("aspect_ratio", &Camera::aspectRatio)
        .def_readwrite("focal_length", &Camera::focalLength)
        .def_readwrite("aperture", &Camera::aperture)
        .def_readwrite("focus_distance", &Camera::focusDistance)
        .def_readwrite("is_perspective", &Camera::isPerspective);

    pybind11::class_<UnrealLiveLink_LightStatic>(m, "LightStatic")
        .def(pybind11::init<>([]() {
            auto ls = UnrealLiveLink_LightStatic();
            UnrealLiveLink_InitLightStatic(&ls);
            return ls;
        }))
        .def_readwrite("is_temperature_supported", &UnrealLiveLink_LightStatic::isTemperatureSupported)
        .def_readwrite("is_intensity_supported", &UnrealLiveLink_LightStatic::isIntensitySupported)
        .def_readwrite("is_light_color_supported", &UnrealLiveLink_LightStatic::isLightColorSupported)
        .def_readwrite("is_inner_cone_angle_supported", &UnrealLiveLink_LightStatic::isInnerConeAngleSupported)
        .def_readwrite("is_outer_cone_angle_supported", &UnrealLiveLink_LightStatic::isOuterConeAngleSupported)
        .def_readwrite("is_attenuation_radius_supported", &UnrealLiveLink_LightStatic::isAttenuationRadiusSupported)
        .def_readwrite("is_source_length_supported", &UnrealLiveLink_LightStatic::isSourceLengthSupported)
        .def_readwrite("is_source_radius_supported", &UnrealLiveLink_LightStatic::isSourceRadiusSupported)
        .def_readwrite("is_soft_source_radius_supported", &UnrealLiveLink_LightStatic::isSoftSourceRadiusSupported);

    pybind11::class_<Light>(m, "Light")
        .def(pybind11::init<>())
        .def_readwrite("transform", &Light::transform)
        .def_readwrite("temperature", &Light::temperature)
        .def_readwrite("intensity", &Light::intensity)
        .def_readwrite("light_color", &Light::lightColor)
        .def_readwrite("inner_cone_angle", &Light::innerConeAngle)
        .def_readwrite("outer_cone_angle", &Light::outerConeAngle)
        .def_readwrite("attenuation_radius", &Light::attenuationRadius)
        .def_readwrite("source_radius", &Light::sourceRadius)
        .def_readwrite("soft_source_radius", &Light::softSourceRadius)
        .def_readwrite("source_length", &Light::sourceLength);

    pybind11::class_<Transform>(m, "Transform")
        .def(pybind11::init<>())
        .def_readwrite("rotation", &Transform::rotation)
        .def_readwrite("translation", &Transform::translation)
        .def_readwrite("scale", &Transform::scale);

    pybind11::class_<KeyValue>(m, "KeyValue")
        .def(pybind11::init<>())
        .def_readwrite("key", &KeyValue::key)
        .def_readwrite("value", &KeyValue::value);

    pybind11::class_<Metadata>(m, "Metadata")
        .def(pybind11::init<>())
        .def_readwrite("key_values", &Metadata::keyValues)
        .def_readwrite("timecode", &Metadata::timecode);

    pybind11::class_<Bone>(m, "Bone")
        .def(pybind11::init<>())
        .def_readwrite("name", &Bone::name)
        .def_readwrite("parent_index", &Bone::parentIndex);

    py::bind_vector<std::vector<std::string>>(m, "Properties");
    py::bind_vector<std::vector<float>>(m, "PropertyValues");

    py::bind_vector<std::vector<Bone>>(m, "AnimationStatic");
    py::bind_vector<std::vector<Transform>>(m, "Animation");

    m.def("load", []() -> int { 
#ifdef WIN32
        const char* sharedObj = "UnrealLiveLinkCInterface.dll";
#else
        const char* sharedObj = "UnrealLiveLinkCInterface.so";
#endif
        return UnrealLiveLink_Load(sharedObj);
    });
    m.def("is_loaded", []() -> bool { return UnrealLiveLink_IsLoaded() == UNREAL_LIVE_LINK_OK; });
    m.def("unload", []() -> void { UnrealLiveLink_Unload(); });

    m.def("set_provider_name", [](const std::string& provider_name) -> void { 
        if (UnrealLiveLink_SetProviderName != NULL) {
            UnrealLiveLink_SetProviderName(provider_name.c_str());
        }
    });

    m.def("get_version", []() -> int { return UnrealLiveLink_GetVersion != NULL ? UnrealLiveLink_GetVersion() : 0 ; });
    m.def("has_connection", []() -> bool { return UnrealLiveLink_HasConnection != NULL ? UnrealLiveLink_HasConnection() == UNREAL_LIVE_LINK_OK : false; });

    m.def("set_unicast_endpoint", [](const std::string& endpoint) -> void { 
        if (UnrealLiveLink_SetUnicastEndpoint != NULL) {
            UnrealLiveLink_SetUnicastEndpoint(endpoint.c_str());
        }
    });
    m.def("add_static_endpoint", [](const std::string& endpoint) -> int { 
        if (UnrealLiveLink_AddStaticEndpoint != NULL) {
            return UnrealLiveLink_AddStaticEndpoint(endpoint.c_str());
        }
        return UNREAL_LIVE_LINK_NOT_LOADED;
    });
    m.def("remove_static_endpoint", [](const std::string& endpoint) -> int { 
        if (UnrealLiveLink_RemoveStaticEndpoint != NULL) {
            return UnrealLiveLink_RemoveStaticEndpoint(endpoint.c_str());
        }
        return UNREAL_LIVE_LINK_NOT_LOADED;
    });

    m.def("start_live_link", []() -> int { return UnrealLiveLink_StartLiveLink != NULL ? UnrealLiveLink_StartLiveLink() : UNREAL_LIVE_LINK_NOT_LOADED ; });
    m.def("stop_live_link", []() -> int { return UnrealLiveLink_StopLiveLink != NULL ? UnrealLiveLink_StopLiveLink() : UNREAL_LIVE_LINK_NOT_LOADED ; });

    m.def("set_basic_structure", &SetBasicStructure);
    m.def("update_basic_frame", &UpdateBasicFrame);
    m.def("set_transform_structure", &SetTransformStructure);
    m.def("update_transform_frame", &UpdateTransformFrame);
    m.def("set_animation_structure", &SetAnimationStructure);
    m.def("update_animation_frame", &UpdateAnimationFrame);
    m.def("set_camera_structure", &SetCameraStructure);
    m.def("update_camera_frame", &UpdateCameraFrame);
    m.def("set_light_structure", &SetLightStructure);
    m.def("update_light_frame", &UpdateLightFrame);

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}


