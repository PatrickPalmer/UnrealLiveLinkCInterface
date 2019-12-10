/** 
 * Copyright (c) 2020 Patrick Palmer, The Jim Henson Company.
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

/**
 * References:
 *     * https://docs.unrealengine.com/en-US/Engine/Animation/LiveLinkPlugin/index.html
 *     * https://github.com/ue4plugins/MayaLiveLink
 *     * http://www.cplusplus.com/articles/48TbqMoL
 */ 

#include "UnrealLiveLinkCInterfaceAPI.h"

#include <stddef.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#else
#include <dlfcn.h>
#endif



/* function pointers */
int (*UnrealLiveLink_GetVersion)() = NULL;
bool (*UnrealLiveLink_InitializeMessagingInterface)(const char *) = NULL;
bool (*UnrealLiveLink_UninitializeMessagingInterface)() = NULL;

void (*UnrealLiveLink_RegisterConnectionUpdateCallback)(void (*callback)()) = NULL;
bool (*UnrealLiveLink_HasConnection)() = NULL;

void (*UnrealLiveLink_SetBasicStructure)(const char *subjectName, const UnrealLiveLink_Properties *properties) = NULL;
void (*UnrealLiveLink_UpdateBasicFrame)(const char *subjectName, const double worldTime, const UnrealLiveLink_Metadata *metadata,
	const UnrealLiveLink_PropertyValues *propValues) = NULL;

void (*UnrealLiveLink_DefaultAnimationStructure)(const char *name, const UnrealLiveLink_Properties *properties) = NULL;
void (*UnrealLiveLink_SetAnimationStructure)(
	const char *subjectName, const UnrealLiveLink_Properties *properties, UnrealLiveLink_AnimationStatic *structure) = NULL;
void (*UnrealLiveLink_UpdateAnimationFrame)(const char *subjectName, const double worldTime,
	const UnrealLiveLink_Metadata *metadata, const UnrealLiveLink_PropertyValues *propValues,
	const UnrealLiveLink_Animation *frame) = NULL;

void (*UnrealLiveLink_SetTransformStructure)(const char *subjectName, const UnrealLiveLink_Properties *properties) = NULL;
void (*UnrealLiveLink_UpdateTransformFrame)(const char *subjectName, const double worldTime,
	const UnrealLiveLink_Metadata *metadata, const UnrealLiveLink_PropertyValues *propValues,
	const UnrealLiveLink_Transform *frame) = NULL;

void (*UnrealLiveLink_SetCameraStructure)(
	const char *subjectName, const UnrealLiveLink_Properties *properties, UnrealLiveLink_CameraStatic *structure) = NULL;
void (*UnrealLiveLink_UpdateCameraFrame)(const char *subjectName, const double worldTime, const UnrealLiveLink_Metadata *metadata,
	const UnrealLiveLink_PropertyValues *propValues, const UnrealLiveLink_Camera *frame) = NULL;

void (*UnrealLiveLink_SetLightStructure)(
	const char *subjectName, const UnrealLiveLink_Properties *properties, UnrealLiveLink_LightStatic *structure) = NULL;
void (*UnrealLiveLink_UpdateLightFrame)(const char *subjectName, const double worldTime, const UnrealLiveLink_Metadata *metadata,
	const UnrealLiveLink_PropertyValues *propValues, const UnrealLiveLink_Light *frame) = NULL;

#ifdef WIN32
static HMODULE UnrealLiveLink_SharedObject = NULL;

#define GET_FUNC_ADDR GetProcAddress
#else
static void * UnrealLiveLink_SharedObject = NULL;

#define GET_FUNC_ADDR dlsym
#endif

int UnrealLiveLink_Load(const char *cInterfaceSharedObjectFilename, const char *interfaceName)
{
	UnrealLiveLink_SharedObject = NULL;

#ifdef WIN32
	HMODULE mod = LoadLibrary(cInterfaceSharedObjectFilename);
#else
	void * mod = dlopen(cInterfaceSharedObjectFilename, RTLD_LAZY);
#endif
	if (!mod)
	{
		return UNREAL_LIVE_LINK_MISSING_LIB;
	}

	/* check API version */
	UnrealLiveLink_GetVersion = (int (*)()) GET_FUNC_ADDR(mod, "UnrealLiveLink_GetVersion");
	if (UnrealLiveLink_GetVersion)
	{
		if ((*UnrealLiveLink_GetVersion)() != UNREAL_LIVE_LINK_API_VERSION)
		{
			return UNREAL_LIVE_LINK_WRONG_VERSION;
		}
	}
	else
	{
		return UNREAL_LIVE_LINK_INCOMPLETE;
	}

	UnrealLiveLink_InitializeMessagingInterface =
		(bool (*)(const char *)) GET_FUNC_ADDR(mod, "UnrealLiveLink_InitializeMessagingInterface");
	UnrealLiveLink_UninitializeMessagingInterface =
		(bool (*)()) GET_FUNC_ADDR(mod, "UnrealLiveLink_UninitializeMessagingInterface");
	UnrealLiveLink_RegisterConnectionUpdateCallback =
		(void (*)(void (*)())) GET_FUNC_ADDR(mod, "UnrealLiveLink_RegisterConnectionUpdateCallback");
	UnrealLiveLink_HasConnection = (bool (*)()) GET_FUNC_ADDR(mod, "UnrealLiveLink_HasConnection");

	UnrealLiveLink_SetBasicStructure =
		(void (*)(const char *, const UnrealLiveLink_Properties *)) GET_FUNC_ADDR(mod, "UnrealLiveLink_SetBasicStructure");
	UnrealLiveLink_UpdateBasicFrame = (void (*)(const char *, const double, const UnrealLiveLink_Metadata *,
		const UnrealLiveLink_PropertyValues *)) GET_FUNC_ADDR(mod, "UnrealLiveLink_UpdateBasicFrame");

	UnrealLiveLink_DefaultAnimationStructure =
		(void (*)(const char *, const UnrealLiveLink_Properties *)) GET_FUNC_ADDR(mod, "UnrealLiveLink_DefaultAnimationStructure");
	UnrealLiveLink_SetAnimationStructure = (void (*)(const char *, const UnrealLiveLink_Properties *,
		UnrealLiveLink_AnimationStatic *)) GET_FUNC_ADDR(mod, "UnrealLiveLink_SetAnimationStructure");
	UnrealLiveLink_UpdateAnimationFrame =
		(void (*)(const char *, const double, const UnrealLiveLink_Metadata *, const UnrealLiveLink_PropertyValues *,
			const UnrealLiveLink_Animation *)) GET_FUNC_ADDR(mod, "UnrealLiveLink_UpdateAnimationFrame");

	UnrealLiveLink_SetTransformStructure =
		(void (*)(const char *, const UnrealLiveLink_Properties *)) GET_FUNC_ADDR(mod, "UnrealLiveLink_SetTransformStructure");
	UnrealLiveLink_UpdateTransformFrame =
		(void (*)(const char *, const double, const UnrealLiveLink_Metadata *, const UnrealLiveLink_PropertyValues *,
			const UnrealLiveLink_Transform *)) GET_FUNC_ADDR(mod, "UnrealLiveLink_UpdateTransformFrame");

	UnrealLiveLink_SetCameraStructure = (void (*)(const char *, const UnrealLiveLink_Properties *,
		UnrealLiveLink_CameraStatic *)) GET_FUNC_ADDR(mod, "UnrealLiveLink_SetCameraStructure");
	UnrealLiveLink_UpdateCameraFrame =
		(void (*)(const char *, const double, const UnrealLiveLink_Metadata *, const UnrealLiveLink_PropertyValues *,
			const UnrealLiveLink_Camera *)) GET_FUNC_ADDR(mod, "UnrealLiveLink_UpdateCameraFrame");

	UnrealLiveLink_SetLightStructure = (void (*)(const char *, const UnrealLiveLink_Properties *,
		UnrealLiveLink_LightStatic *)) GET_FUNC_ADDR(mod, "UnrealLiveLink_SetLightStructure");
	UnrealLiveLink_UpdateLightFrame =
		(void (*)(const char *, const double, const UnrealLiveLink_Metadata *, const UnrealLiveLink_PropertyValues *,
			const UnrealLiveLink_Light *)) GET_FUNC_ADDR(mod, "UnrealLiveLink_UpdateLightFrame");

	if (!UnrealLiveLink_InitializeMessagingInterface || !UnrealLiveLink_UninitializeMessagingInterface ||
		!UnrealLiveLink_RegisterConnectionUpdateCallback || !UnrealLiveLink_HasConnection || !UnrealLiveLink_SetBasicStructure ||
		!UnrealLiveLink_UpdateBasicFrame || !UnrealLiveLink_DefaultAnimationStructure || !UnrealLiveLink_SetAnimationStructure ||
		!UnrealLiveLink_UpdateAnimationFrame || !UnrealLiveLink_SetTransformStructure || !UnrealLiveLink_UpdateTransformFrame ||
		!UnrealLiveLink_SetCameraStructure || !UnrealLiveLink_UpdateCameraFrame || !UnrealLiveLink_SetLightStructure ||
		!UnrealLiveLink_UpdateLightFrame)
	{
		return UNREAL_LIVE_LINK_INCOMPLETE;
	}

	UnrealLiveLink_SharedObject = mod;

	UnrealLiveLink_InitializeMessagingInterface(interfaceName);

	return UNREAL_LIVE_LINK_OK;
}

void UnrealLiveLink_Unload()
{
	if (UnrealLiveLink_UninitializeMessagingInterface)
	{
		UnrealLiveLink_UninitializeMessagingInterface();
	}

	if (UnrealLiveLink_SharedObject)
	{
#ifdef WIN32
		FreeLibrary(UnrealLiveLink_SharedObject);
#else
		dlclose(UnrealLiveLink_SharedObject);
#endif
		UnrealLiveLink_SharedObject = NULL;
	}
}

bool UnrealLiveLink_IsLoaded()
{
	return UnrealLiveLink_SharedObject != NULL;
}

void UnrealLiveLink_InitMetadata(UnrealLiveLink_Metadata *metadata)
{
	metadata->keyValueCount = 0;
	metadata->keyValues = 0;

	metadata->timecode.hours = 0;
	metadata->timecode.minutes = 0;
	metadata->timecode.seconds = 0;
	metadata->timecode.frames = 0;
	metadata->timecode.format = UNREAL_LIVE_LINK_TIMECODE_UNKNOWN;
}

void UnrealLiveLink_InitTransform(UnrealLiveLink_Transform * transform)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		transform->rotation[i] = 0.0f;
	}
	for (i = 0; i < 3; i++)
	{
		transform->translation[i] = 0.0f;
		transform->scale[i] = 1.0f;
	}
}

void UnrealLiveLink_InitCameraStatic(UnrealLiveLink_CameraStatic *structure)
{
	structure->isFieldOfViewSupported = false;
	structure->isAspectRatioSupported = false;
	structure->isFocalLengthSupported = false;
	structure->isProjectionModeSupported = false;
	structure->filmBackWidth = -1.0f;
	structure->filmBackHeight = -1.0f;
	structure->isApertureSupported = false;
	structure->isFocusDistanceSupported = false;
}

void UnrealLiveLink_InitCamera(UnrealLiveLink_Camera *structure)
{
	UnrealLiveLink_InitTransform(&(structure->transform));

	structure->fieldOfView = 90.f;
	structure->aspectRatio = 1.777778f;
	structure->focalLength = 50.f;
	structure->aperture = 2.8f;
	structure->focusDistance = 100000.0f;
	structure->isPerspective = true;
}

void UnrealLiveLink_InitLightStatic(UnrealLiveLink_LightStatic *structure)
{
	structure->isTemperatureSupported = false;
	structure->isIntensitySupported = false;
	structure->isLightColorSupported = false;
	structure->isInnerConeAngleSupported = false;
	structure->isOuterConeAngleSupported = false;
	structure->isAttenuationRadiusSupported = false;
	structure->isSourceLengthSupported = false;
	structure->isSourceRadiusSupported = false;
	structure->isSoftSourceRadiusSupported = false;
}

void UnrealLiveLink_InitLight(UnrealLiveLink_Light *structure)
{
	int i;

	UnrealLiveLink_InitTransform(&(structure->transform));

	structure->temperature = 6500.0f;
	structure->intensity = 3.1415926535897932f;

	structure->innerConeAngle = 0.0f;
	structure->outerConeAngle = 44.0f;
	structure->attenuationRadius = 1000.0f;
	structure->sourceRadius = 0.0f;
	structure->softSourceRadius = 0.0f;
	structure->sourceLength = 0.0f;

	/* default white */
	for (i = 0; i < 4; i++)
	{
		structure->lightColor[i] = 255;
	}
}
