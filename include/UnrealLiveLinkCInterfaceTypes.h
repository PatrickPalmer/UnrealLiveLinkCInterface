
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

#ifndef _UNREAL_LIVE_LINK_C_INTERFACE_TYPES_H
#define _UNREAL_LIVE_LINK_C_INTERFACE_TYPES_H 1


#include <stdint.h>
#include <stdbool.h>

#define UNREAL_LIVE_LINK_API_VERSION 2

#define UNREAL_LIVE_LINK_MAX_NAME_LENGTH 128

typedef char UnrealLiveLink_Name[UNREAL_LIVE_LINK_MAX_NAME_LENGTH];

/* transformation
 * rotation is quaternion, w is 4th field
 */
struct UnrealLiveLink_Transform
{
	float rotation[4];
	float translation[3];
	float scale[3];
};

/**
 * timecode formats
 * SMPTE standards: SMPTE ST12-1 p6, SMPTE ST428-11:2013 p3
 */ 
enum UnrealLiveLink_TimecodeFormat
{
	UNREAL_LIVE_LINK_TIMECODE_UNKNOWN = 0,
	
	UNREAL_LIVE_LINK_TIMECODE_23_98,
	UNREAL_LIVE_LINK_TIMECODE_24,
	UNREAL_LIVE_LINK_TIMECODE_25,
	UNREAL_LIVE_LINK_TIMECODE_29_97_NDF,
	UNREAL_LIVE_LINK_TIMECODE_29_97_DF,
	UNREAL_LIVE_LINK_TIMECODE_30,
	UNREAL_LIVE_LINK_TIMECODE_47_95,
	UNREAL_LIVE_LINK_TIMECODE_48,
	UNREAL_LIVE_LINK_TIMECODE_50,
	UNREAL_LIVE_LINK_TIMECODE_59_94_NDF,
	UNREAL_LIVE_LINK_TIMECODE_59_94_DF,
	UNREAL_LIVE_LINK_TIMECODE_60,
	UNREAL_LIVE_LINK_TIMECODE_72,
	UNREAL_LIVE_LINK_TIMECODE_96,
	UNREAL_LIVE_LINK_TIMECODE_100,
	UNREAL_LIVE_LINK_TIMECODE_120
};

struct UnrealLiveLink_Timecode
{
	int32_t hours;
	int32_t minutes;
	int32_t seconds;
	int32_t frames;
	enum UnrealLiveLink_TimecodeFormat format;
};

/* property names */
struct UnrealLiveLink_Properties
{
	UnrealLiveLink_Name *names;
	int nameCount;
};

/* property values (array values match the Property Names index) */
struct UnrealLiveLink_PropertyValues
{
	float *values;
	int valueCount;
};

struct UnrealLiveLink_KeyValue
{
	UnrealLiveLink_Name name;
	UnrealLiveLink_Name value;
};

struct UnrealLiveLink_Metadata
{
	/* string map */
	struct UnrealLiveLink_KeyValue *keyValues;
	int keyValueCount;

	/* frame timecode */
	struct UnrealLiveLink_Timecode timecode;
};

struct UnrealLiveLink_Bone
{
	/* bone name */
	UnrealLiveLink_Name name;

	/* bone parent index, use parentIndex=-1 for root */
	int parentIndex;
};

struct UnrealLiveLink_AnimationStatic
{
	/* list of bone names and their parents */
	struct UnrealLiveLink_Bone *bones;
	int boneCount;
};

struct UnrealLiveLink_Animation
{
	/* bone transforms */
	struct UnrealLiveLink_Transform *transforms;
	int transformCount;
};

struct UnrealLiveLink_CameraStatic
{
	/* whether to use field of view per frame */
	bool isFieldOfViewSupported;

	/* whether to use aspect ratio per frame */
	bool isAspectRatioSupported;

	/* whether to use focal length per frame */
	bool isFocalLengthSupported;

	/* whether to use projection mode per frame */
	bool isProjectionModeSupported;

	/* film back, only for cinematic camera, values greater than 0 will be applied */
	float filmBackWidth;
	float filmBackHeight;

	/* whether to use aperture per frame */
	bool isApertureSupported;

	/* whether to use focus distance per frame */
	bool isFocusDistanceSupported;
};

struct UnrealLiveLink_Camera
{
	/* transform */
	struct UnrealLiveLink_Transform transform;

	/* Field of View of the camera in degrees */
	float fieldOfView;

	/* Aspect Ratio of the camera (Width / Height) */
	float aspectRatio;

	/* Focal length of the camera */
	float focalLength;

	/* Aperture of the camera in terms of f-stop */
	float aperture;

	/* Focus distance of the camera in cm. Works only in manual focus method */
	float focusDistance;

	/* projection mode of the camera */
	/* true is perspective, false is orthographic */
	bool isPerspective;
};

struct UnrealLiveLink_LightStatic
{
	/* whether to use temperature per frame */
	bool isTemperatureSupported;

	/* whether to use intensity per frame */
	bool isIntensitySupported;

	/* whether to use light color per frame */
	bool isLightColorSupported;

	/* whether to use inner cone angle per frame, spotlight only */
	bool isInnerConeAngleSupported;

	/* whether to use outer cone angle per frame, spotlight only */
	bool isOuterConeAngleSupported;

	/* whether to use attenuation radius per frame */
	bool isAttenuationRadiusSupported;

	/* whether to use source length per frame */
	bool isSourceLengthSupported;

	/* whether to use source radius per frame */
	bool isSourceRadiusSupported;

	/* whether to use soft source radius per frame */
	bool isSoftSourceRadiusSupported;
};

struct UnrealLiveLink_Light
{
	/* transform */
	struct UnrealLiveLink_Transform transform;

	/* color temperature in Kelvin of the blackbody illuminant */
	float temperature;

	/* total energy that the light emits in lux. */
	float intensity;

	/* filter color of the light. */
	uint8_t lightColor[4];

	/* inner cone angle in degrees for a Spotlight. */
	float innerConeAngle;

	/* outer cone angle in degrees for a Spotlight. */
	float outerConeAngle;

	/* light visible influence. Works for Pointlight and Spotlight. */
	float attenuationRadius;

	/* radius of light source shape. Works for Pointlight and Spotlight. */
	float sourceRadius;

	/* soft radius of light source shape. Works for Pointlight and Spotlight. */
	float softSourceRadius;

	/* length of light source shape. Works for Pointlight and Spotlight. */
	float sourceLength;
};

#endif

