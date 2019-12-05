
#pragma once

#include "UnrealLiveLinkCInterfaceTypes.h"

#ifdef WIN32
#define APICALL __declspec(dllexport)
#else
#define APICALL
#endif


#ifdef __cplusplus
extern "C"
{
#endif

APICALL int UnrealLiveLink_GetVersion();

APICALL bool UnrealLiveLink_InitializeMessagingInterface(const char *interfaceName);

APICALL bool UnrealLiveLink_UninitializeMessagingInterface();

APICALL void UnrealLiveLink_RegisterConnectionUpdateCallback(void (*callback)());

APICALL bool UnrealLiveLink_HasConnection();

APICALL void UnrealLiveLink_SetBasicStructure(const char *subjectName, const UnrealLiveLink_Properties *properties);
APICALL void UnrealLiveLink_UpdateBasicFrame(const char *subjectName, const double worldTime,
	const UnrealLiveLink_Metadata *metadata, const UnrealLiveLink_PropertyValues *propValues);

APICALL void UnrealLiveLink_DefaultAnimationStructure(const char *name, const UnrealLiveLink_Properties *properties);
APICALL void UnrealLiveLink_SetAnimationStructure(
	const char *subjectName, const UnrealLiveLink_Properties *properties, UnrealLiveLink_AnimationStatic *structure);
APICALL void UnrealLiveLink_UpdateAnimationFrame(const char *subjectName, const double worldTime,
	const UnrealLiveLink_Metadata *metadata, const UnrealLiveLink_PropertyValues *propValues,
	const UnrealLiveLink_Animation *frame);

APICALL void UnrealLiveLink_SetTransformStructure(const char *subjectName, const UnrealLiveLink_Properties *properties);
APICALL void UnrealLiveLink_UpdateTransformFrame(const char *subjectName, const double worldTime,
	const UnrealLiveLink_Metadata *metadata, const UnrealLiveLink_PropertyValues *propValues,
	const UnrealLiveLink_Transform *frame);

APICALL void UnrealLiveLink_SetCameraStructure(
	const char *subjectName, const UnrealLiveLink_Properties *properties, UnrealLiveLink_CameraStatic *structure);
APICALL void UnrealLiveLink_UpdateCameraFrame(const char *subjectName, const double worldTime,
	const UnrealLiveLink_Metadata *metadata, const UnrealLiveLink_PropertyValues *propValues, const UnrealLiveLink_Camera *frame);

APICALL void UnrealLiveLink_SetLightStructure(
	const char *subjectName, const UnrealLiveLink_Properties *properties, UnrealLiveLink_LightStatic *structure);
APICALL void UnrealLiveLink_UpdateLightFrame(const char *subjectName, const double worldTime,
	const UnrealLiveLink_Metadata *metadata, const UnrealLiveLink_PropertyValues *propValues, const UnrealLiveLink_Light *frame);

#ifdef __cplusplus
}
#endif
