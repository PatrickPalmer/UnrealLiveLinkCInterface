
#pragma once

#include "UnrealLiveLinkCInterfaceTypes.h"

#ifdef WIN32
#define APICALL __declspec(dllexport)
#else
#define APICALL __attribute__((visibility("default")))
#endif


#ifdef __cplusplus
extern "C"
{
#endif

APICALL void UnrealLiveLink_Initialize();
APICALL void UnrealLiveLink_Shutdown();

APICALL int UnrealLiveLink_GetVersion();

APICALL void UnrealLiveLink_SetProviderName(const char *ProviderName);

APICALL int UnrealLiveLink_StartLiveLink();

APICALL int UnrealLiveLink_StopLiveLink();

APICALL void UnrealLiveLink_SetUnicastEndpoint(const char* Endpoint);
APICALL int UnrealLiveLink_AddStaticEndpoint(const char* Endpoint);
APICALL int UnrealLiveLink_RemoveStaticEndpoint(const char* Endpoint);

APICALL void UnrealLiveLink_RegisterConnectionUpdateCallback(void (*Callback)());

APICALL int UnrealLiveLink_HasConnection();

APICALL void UnrealLiveLink_SetBasicStructure(const char *SubjectName, const UnrealLiveLink_Properties *Properties);
APICALL void UnrealLiveLink_UpdateBasicFrame(const char *SubjectName, const double WorldTime,
	const UnrealLiveLink_Metadata *Metadata, const UnrealLiveLink_PropertyValues *PropValues);

APICALL void UnrealLiveLink_SetAnimationStructure(
	const char *SubjectName, const UnrealLiveLink_Properties *Properties, UnrealLiveLink_AnimationStatic *AnimStructure);
APICALL void UnrealLiveLink_UpdateAnimationFrame(const char *SubjectName, const double WorldTime,
	const UnrealLiveLink_Metadata *Metadata, const UnrealLiveLink_PropertyValues *PropValues,
	const UnrealLiveLink_Animation *Frame);

APICALL void UnrealLiveLink_SetTransformStructure(const char *SubjectName, const UnrealLiveLink_Properties *Properties);
APICALL void UnrealLiveLink_UpdateTransformFrame(const char *SubjectName, const double WorldTime,
	const UnrealLiveLink_Metadata *Metadata, const UnrealLiveLink_PropertyValues *PropValues,
	const UnrealLiveLink_Transform *Frame);

APICALL void UnrealLiveLink_SetCameraStructure(
	const char *SubjectName, const UnrealLiveLink_Properties *Properties, UnrealLiveLink_CameraStatic *CameraStructure);
APICALL void UnrealLiveLink_UpdateCameraFrame(const char *SubjectName, const double WorldTime,
	const UnrealLiveLink_Metadata *Metadata, const UnrealLiveLink_PropertyValues *PropValues, const UnrealLiveLink_Camera *Frame);

APICALL void UnrealLiveLink_SetLightStructure(
	const char *SubjectName, const UnrealLiveLink_Properties *Properties, UnrealLiveLink_LightStatic *LightStructure);
APICALL void UnrealLiveLink_UpdateLightFrame(const char *SubjectName, const double WorldTime,
	const UnrealLiveLink_Metadata *Metadata, const UnrealLiveLink_PropertyValues *PropValues, const UnrealLiveLink_Light *Frame);

#ifdef __cplusplus
}
#endif
