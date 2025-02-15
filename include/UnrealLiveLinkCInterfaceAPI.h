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

#ifndef _UNREAL_LIVE_LINK_C_INTERFACE_API_H
#define _UNREAL_LIVE_LINK_C_INTERFACE_API_H 1

#include "UnrealLiveLinkCInterfaceTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * load the Unreal Live Link C Interface shared object
 * for the load to succeed, the Unreal Live Link C Interface version must match
 * @param cInterfaceSharedObjectFilename shared object filename
 * @return results (success returns UNREAL_LIVE_LINK_OK)
 */
extern int UnrealLiveLink_Load(const char *cInterfaceSharedObjectFilename);

/**
 * if Unreal Live Link C Interface shared object is loaded
 * @returns UNREAL_LIVE_LINK_OK if loaded, UNREAD_LIVE_LINK_NOT_LOADED if not
 */
extern int UnrealLiveLink_IsLoaded(void);

/**
 * unload Unreal Live Link C Interface shared object
 */
extern void UnrealLiveLink_Unload(void);

/**
 * set the name of the Message Bus Provider
 * @param providerName name of Live Link interface appearing in Unreal
 */
extern void (*UnrealLiveLink_SetProviderName)(const char *providerName);

/**
 * get the current version of the Unreal Live Link C Interface
 * the version integer is specific to this API and not matching against Unreal Versions
 * @return version of the Unreal Live Link C Interface (UNREAL_LIVE_LINK_API_VERSION value at compile time)
 */
extern int (*UnrealLiveLink_GetVersion)(void);

/**
 * start live link connection
 * @return results (success returns UNREAL_LIVE_LINK_OK)
 */
extern int (*UnrealLiveLink_StartLiveLink)(void);

/**
 * stop live link connection
 * @return results (success returns UNREAL_LIVE_LINK_OK)
 */
extern int (*UnrealLiveLink_StopLiveLink)(void);

/**
 * set a unicast endpoint
 * @return results (success returns UNREAL_LIVE_LINK_OK)
 */
extern void (*UnrealLiveLink_SetUnicastEndpoint)(const char* Endpoint);

/**
 * add a static endpoint
 * @return results (success returns UNREAL_LIVE_LINK_OK)
 */
extern int (*UnrealLiveLink_AddStaticEndpoint)(const char* Endpoint);

/**
 * remove a static endpoint
 * @return results (success returns UNREAL_LIVE_LINK_OK)
 */
extern int (*UnrealLiveLink_RemoveStaticEndpoint)(const char* Endpoint);

/**
 * register a function callback if the connection to Unreal changes
 * @param callback callback function
 */
extern void (*UnrealLiveLink_RegisterConnectionUpdateCallback)(void (*callback)());

/**
 * connection with Unreal?
 * @return UNREAL_LIVE_LINK_OK if connected, UNREAD_LIVE_LINK_NOT_CONNECTED if not
 */
extern int (*UnrealLiveLink_HasConnection)();

/**
 * initialize the Metadata structure with default values
 * @param metadata Metadata structure
 */
extern void UnrealLiveLink_InitMetadata(struct UnrealLiveLink_Metadata *metadata);




/** Basic Generic Roll **/

/**
 * Basic Generic Roll setup
 * @param subjectName Unreal subject name
 * @param properties named float properties
 */
extern void (*UnrealLiveLink_SetBasicStructure)(const char *subjectName, const struct UnrealLiveLink_Properties *properties);

/**
 * Basic Generic Roll per frame values
 * @param subjectName Unreal subject name
 * @param worldTime frame time
 * @param metadata associated metadata (may pass in null for none)
 * @param propValues named properties float values (may pass in null for none)
 */
extern void (*UnrealLiveLink_UpdateBasicFrame)(const char *subjectName, const double worldTime,
	const struct UnrealLiveLink_Metadata *metadata, const struct UnrealLiveLink_PropertyValues *propValues);



/** Animation Roll **/

/**
 * Animation Roll setup
 * @param subjectName Unreal subject name
 * @param properties named float properties
 */
extern void (*UnrealLiveLink_SetAnimationStructure)(
	const char *subjectName, const struct UnrealLiveLink_Properties *properties, struct UnrealLiveLink_AnimationStatic *structure);

/**
 * Animation Roll per frame values
 * @param subjectName Unreal subject name
 * @param worldTime frame time
 * @param metadata associated metadata (may pass in null for none)
 * @param propValues named properties float values (may pass in null for none)
 * @param frame animation frame
 */
extern void (*UnrealLiveLink_UpdateAnimationFrame)(const char *subjectName, const double worldTime,
	const struct UnrealLiveLink_Metadata *metadata, const struct UnrealLiveLink_PropertyValues *propValues,
	const struct UnrealLiveLink_Animation *frame);



/** Transform Roll **/

/**
 * initialize transform structure to identity
 * @param transform transform to set
 */
extern void UnrealLiveLink_InitTransform(struct UnrealLiveLink_Transform *transform);

/**
 * Transform Roll setup
 * @param subjectName Unreal subject name
 * @param properties named float properties
 */
extern void (*UnrealLiveLink_SetTransformStructure)(const char *subjectName, const struct UnrealLiveLink_Properties *properties);

/**
 * Transform Roll per frame values
 * @param subjectName Unreal subject name
 * @param worldTime frame time
 * @param metadata associated metadata (may pass in null for none)
 * @param propValues named properties float values (may pass in null for none)
 * @param frame transform
 */
extern void (*UnrealLiveLink_UpdateTransformFrame)(const char *subjectName, const double worldTime,
	const struct UnrealLiveLink_Metadata *metadata, const struct UnrealLiveLink_PropertyValues *propValues,
	const struct UnrealLiveLink_Transform *frame);



/** Camera Roll **/

/**
 * initialize camera static to default values
 * @param structure structure to set
 */
extern void UnrealLiveLink_InitCameraStatic(struct UnrealLiveLink_CameraStatic *structure);

/**
 * initialize camera per frame to default values
 * @param structure structure to set
 */
extern void UnrealLiveLink_InitCamera(struct UnrealLiveLink_Camera *structure);

/**
 * Camera Roll setup
 * @param subjectName Unreal subject name
 * @param properties named float properties
 */
extern void (*UnrealLiveLink_SetCameraStructure)(
	const char *subjectName, const struct UnrealLiveLink_Properties *properties, struct UnrealLiveLink_CameraStatic *structure);

/**
 * Camera Roll per frame values
 * @param subjectName Unreal subject name
 * @param worldTime frame time
 * @param metadata associated metadata (may pass in null for none)
 * @param propValues named properties float values (may pass in null for none)
 * @param frame camera values
 */
extern void (*UnrealLiveLink_UpdateCameraFrame)(const char *subjectName, const double worldTime,
	const struct UnrealLiveLink_Metadata *metadata, const struct UnrealLiveLink_PropertyValues *propValues, const struct UnrealLiveLink_Camera *frame);



/** Light Roll **/

/**
 * initialize light static to default values
 * @param structure structure to set
 */
extern void UnrealLiveLink_InitLightStatic(struct UnrealLiveLink_LightStatic *structure);

/**
 * initialize light per frame to default values
 * @param structure structure to set
 */
extern void UnrealLiveLink_InitLight(struct UnrealLiveLink_Light *structure);

/**
 * Light Roll setup
 * @param subjectName Unreal subject name
 * @param properties named float properties
 */
extern void (*UnrealLiveLink_SetLightStructure)(
	const char *subjectName, const struct UnrealLiveLink_Properties *properties, struct UnrealLiveLink_LightStatic *structure);

/**
 * Light Roll per frame values
 * @param subjectName Unreal subject name
 * @param worldTime frame time
 * @param metadata associated metadata (may pass in null for none)
 * @param propValues named properties float values (may pass in null for none)
 * @param frame light values
 */
extern void (*UnrealLiveLink_UpdateLightFrame)(const char *subjectName, const double worldTime,
	const struct UnrealLiveLink_Metadata *metadata, const struct UnrealLiveLink_PropertyValues *propValues, const struct UnrealLiveLink_Light *frame);

#ifdef __cplusplus
}
#endif

#endif

