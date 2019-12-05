
#include "UnrealLiveLinkCInterface.h"

#include "Async/TaskGraphInterfaces.h"
#include "LiveLinkProvider.h"
#include "LiveLinkRefSkeleton.h"
#include "LiveLinkTypes.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/OutputDevice.h"
#include "Modules/ModuleManager.h"
#include "RequiredProgramMainCPPInclude.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "Roles/LiveLinkCameraRole.h"
#include "Roles/LiveLinkCameraTypes.h"
#include "Roles/LiveLinkLightRole.h"
#include "Roles/LiveLinkLightTypes.h"
#include "Roles/LiveLinkTransformRole.h"
#include "Roles/LiveLinkTransformTypes.h"
#include "UObject/Object.h"

#include <cstdio>
#include <vector>
#include <array>

DEFINE_LOG_CATEGORY_STATIC(LogUnrealLiveLinkCInterface, Log, All);

IMPLEMENT_APPLICATION(UnrealLiveLinkCInterface, "UnrealLiveLinkCInterface");

class FLiveLinkStreamedSubjectManager;

TSharedPtr<ILiveLinkProvider> LiveLinkProvider;
TSharedPtr<FLiveLinkStreamedSubjectManager> LiveLinkStreamManager;
FDelegateHandle ConnectionStatusChangedHandle;

std::vector<void (*)()> ConnectionCallbacks;

std::array<std::pair<int32_t, int32_t>, UNREAL_LIVE_LINK_TIMECODE_120+1> TimecodeRates = {
	{
		{ 0, 0 },			// unknown
		{ 24000, 1001 },	// 23.98
		{ 24, 1 },			// 24
		{ 25, 1 },			// 25
		{ 30000, 1001 },	// 29.97 NDF
		{ 30000, 1001 },	// 29.97 DF
		{ 30, 1 },			// 30
		{ 48000, 1001 },	// 47.95
		{ 48, 1 },			// 48
		{ 50, 1 },			// 50
		{ 60000, 1001 },	// 59.94 NDF
		{ 60000, 1001 },	// 59.94 DF
		{ 60, 1 },			// 60
		{ 72, 1 },			// 72
		{ 96, 1 },			// 96
		{ 100, 1 },			// 100
		{ 120, 1 }			// 120
	}
};

// set FTransform from Unreal Live Link C Interface Transform
static void SetFTransform(FTransform &transform, const UnrealLiveLink_Transform &t)
{
	transform.SetRotation(FQuat(t.rotation[0], t.rotation[1], t.rotation[2], t.rotation[3]));
	transform.SetTranslation(FVector(t.translation[0], t.translation[1], t.translation[2]));
	transform.SetScale3D(FVector(t.scale[0], t.scale[1], t.scale[2]));
}


static void OnConnectionStatusChanged()
{
	for (const auto &cb : ConnectionCallbacks)
	{
		cb();
	}
}

int UnrealLiveLink_GetVersion()
{
	return UNREAL_LIVE_LINK_API_VERSION;
}

bool UnrealLiveLink_InitializeMessagingInterface(const char *interfaceName)
{
	// SET UP
	GEngineLoop.PreInit(TEXT("UnrealLiveLinkCInterface -Messaging"));
	ProcessNewlyLoadedUObjects();

	// Tell the module manager now process newly-loaded UObjects when new C++ modules are loaded
	FModuleManager::Get().StartProcessingNewlyLoadedObjects();

	FModuleManager::Get().LoadModule(TEXT("UdpMessaging"));

	GLog->TearDown();

	LiveLinkProvider = ILiveLinkProvider::CreateLiveLinkProvider(ANSI_TO_TCHAR(interfaceName));
	ConnectionStatusChangedHandle = LiveLinkProvider->RegisterConnStatusChangedHandle(FLiveLinkProviderConnectionStatusChanged::FDelegate::CreateStatic(&OnConnectionStatusChanged));

	// We do not tick the core engine but we need to tick the ticker to make sure the message bus endpoint in LiveLinkProvider is
	// up to date
	FTicker::GetCoreTicker().Tick(1.f);

#ifdef TODO
	LiveLinkStreamManager = MakeShareable(new FLiveLinkStreamedSubjectManager());
#endif

	UE_LOG(LogUnrealLiveLinkCInterface, Display, TEXT("Live Link C Interface Initialized"));

	return true;
}

bool UnrealLiveLink_UninitializeMessagingInterface()
{
	// TEAR DOWN
	UE_LOG(LogUnrealLiveLinkCInterface, Display, TEXT("Live Link C Interface Shutting Down"));

	if (ConnectionStatusChangedHandle.IsValid())
	{
		LiveLinkProvider->UnregisterConnStatusChangedHandle(ConnectionStatusChangedHandle);
		ConnectionStatusChangedHandle.Reset();
	}

	FTicker::GetCoreTicker().Tick(1.f);

	LiveLinkProvider = nullptr;

	return true;
}

void UnrealLiveLink_RegisterConnectionUpdateCallback(void (*callback)())
{
	ConnectionCallbacks.push_back(callback);
}

bool UnrealLiveLink_HasConnection()
{
	return LiveLinkProvider->HasConnection();
}


static void SetBasicFrameParameters(const char *subjectName, const double worldTime,
	const UnrealLiveLink_Metadata *metadata, const UnrealLiveLink_PropertyValues *propValues, FLiveLinkFrameDataStruct &frameData)
{
	FLiveLinkBaseFrameData& baseData = *frameData.Cast<FLiveLinkBaseFrameData>();

	baseData.WorldTime = worldTime;
	
	if (propValues)
	{
		for (int i = 0; i < propValues->valueCount; i++)
		{
			baseData.PropertyValues.Add(propValues->values[i]);
		}
	}

	if (metadata)
	{
		for (int i = 0; i < metadata->keyValueCount; i++)
		{
			baseData.MetaData.StringMetaData[metadata->keyValues[i].name] = metadata->keyValues[i].value;
		}

		// set sceneTime
		bool dropFrame = metadata->timecode.format == UNREAL_LIVE_LINK_TIMECODE_29_97_DF || metadata->timecode.format == UNREAL_LIVE_LINK_TIMECODE_59_94_DF;
		FTimecode tc(metadata->timecode.hours, metadata->timecode.minutes, metadata->timecode.seconds, metadata->timecode.frames, dropFrame);

		int32_t nom = TimecodeRates[metadata->timecode.format].first;
		int32_t denom = TimecodeRates[metadata->timecode.format].second;

		FQualifiedFrameTime ft(tc, FFrameRate(nom, denom));
		baseData.MetaData.SceneTime = ft;
	}
}


void UnrealLiveLink_SetBasicStructure(const char *subjectName, const UnrealLiveLink_Properties *properties)
{
	FLiveLinkStaticDataStruct staticData(FLiveLinkBaseStaticData::StaticStruct());
	FLiveLinkBaseStaticData& baseData = *staticData.Cast<FLiveLinkBaseStaticData>();

	if (properties)
	{
		for (int i = 0; i < properties->nameCount; i++)
		{
			baseData.PropertyNames.Add(properties->names[i]);
		}
	}

	LiveLinkProvider->UpdateSubjectStaticData(subjectName, ULiveLinkBasicRole::StaticClass(), MoveTemp(staticData));
}

void UnrealLiveLink_UpdateBasicFrame(const char *subjectName, const double worldTime,
	const UnrealLiveLink_Metadata *metadata, const UnrealLiveLink_PropertyValues *propValues)
{
	FLiveLinkFrameDataStruct frameData(FLiveLinkBaseFrameData::StaticStruct());

	SetBasicFrameParameters(subjectName, worldTime, metadata, propValues, frameData);

	LiveLinkProvider->UpdateSubjectFrameData(subjectName, MoveTemp(frameData));
}


void UnrealLiveLink_DefaultAnimationStructure(const char *name, const UnrealLiveLink_Properties *properties)
{
	UnrealLiveLink_Bone bone;
	::strncpy(bone.name, "root", UNREAL_LIVE_LINK_MAX_NAME_LENGTH);
	bone.parentIndex = -1;

	UnrealLiveLink_AnimationStatic structure;
	structure.boneCount = 1;
	structure.bones = &bone;

	UnrealLiveLink_SetAnimationStructure(name, properties, &structure);
}

void UnrealLiveLink_SetAnimationStructure(
	const char *subjectName, const UnrealLiveLink_Properties *properties, UnrealLiveLink_AnimationStatic *structure)
{
	FLiveLinkStaticDataStruct staticData(FLiveLinkSkeletonStaticData::StaticStruct());
	FLiveLinkSkeletonStaticData& animData = *staticData.Cast<FLiveLinkSkeletonStaticData>();

	if (properties)
	{
		for (int i = 0; i < properties->nameCount; i++)
		{
			animData.PropertyNames.Add(properties->names[i]);
		}
	}

	TArray<FName> names;
	TArray<int32> indices;
	for (int i = 0; i < structure->boneCount; i++)
	{
		names.Add(structure->bones[i].name);
		indices.Add(structure->bones[i].parentIndex);
	}

	animData.SetBoneNames(names);
	animData.SetBoneParents(indices);

	LiveLinkProvider->UpdateSubjectStaticData(subjectName, ULiveLinkAnimationRole::StaticClass(), MoveTemp(staticData));
}

void UnrealLiveLink_UpdateAnimationFrame(const char *subjectName, const double worldTime,
	const UnrealLiveLink_Metadata *metadata, const UnrealLiveLink_PropertyValues *propValues,
	const UnrealLiveLink_Animation *frame)
{
	FLiveLinkFrameDataStruct frameData(FLiveLinkAnimationFrameData::StaticStruct());
	FLiveLinkAnimationFrameData& animData = *frameData.Cast<FLiveLinkAnimationFrameData>();

	for (int i = 0; i < frame->transformCount; i++)
	{
		auto &t = frame->transforms[i];

		FTransform UETrans;

		UETrans.SetRotation(FQuat(t.rotation[0], t.rotation[1], t.rotation[2], t.rotation[3]));

		UETrans.SetTranslation(FVector(t.translation[0], t.translation[1], t.translation[2]));

		UETrans.SetScale3D(FVector(t.scale[0], t.scale[1], t.scale[2]));

		animData.Transforms.Add(UETrans);
	}

	SetBasicFrameParameters(subjectName, worldTime, metadata, propValues, frameData);
	
	LiveLinkProvider->UpdateSubjectFrameData(subjectName, MoveTemp(frameData));
}


void UnrealLiveLink_SetTransformStructure(const char *subjectName, const UnrealLiveLink_Properties *properties)
{
	FLiveLinkStaticDataStruct staticData(FLiveLinkTransformStaticData::StaticStruct());
	FLiveLinkTransformStaticData& xformData = *staticData.Cast<FLiveLinkTransformStaticData>();

	if (properties)
	{
		for (int i = 0; i < properties->nameCount; i++)
		{
			xformData.PropertyNames.Add(properties->names[i]);
		}
	}

	LiveLinkProvider->UpdateSubjectStaticData(subjectName, ULiveLinkTransformRole::StaticClass(), MoveTemp(staticData));
}

void UnrealLiveLink_UpdateTransformFrame(const char *subjectName, const double worldTime,
	const UnrealLiveLink_Metadata *metadata, const UnrealLiveLink_PropertyValues *propValues,
	const UnrealLiveLink_Transform *frame)
{
	FLiveLinkFrameDataStruct frameData(FLiveLinkTransformFrameData::StaticStruct());
	FLiveLinkTransformFrameData& xformData = *frameData.Cast<FLiveLinkTransformFrameData>();

	SetFTransform(xformData.Transform, *frame);

	SetBasicFrameParameters(subjectName, worldTime, metadata, propValues, frameData);
	
	LiveLinkProvider->UpdateSubjectFrameData(subjectName, MoveTemp(frameData));
}


void UnrealLiveLink_SetCameraStructure(
	const char *subjectName, const UnrealLiveLink_Properties *properties, UnrealLiveLink_CameraStatic *structure)
{
	FLiveLinkStaticDataStruct staticData(FLiveLinkCameraStaticData::StaticStruct());
	FLiveLinkCameraStaticData& cameraData = *staticData.Cast<FLiveLinkCameraStaticData>();

	if (properties)
	{
		for (int i = 0; i < properties->nameCount; i++)
		{
			cameraData.PropertyNames.Add(properties->names[i]);
		}
	}
	if (structure)
	{
		cameraData.bIsFieldOfViewSupported = structure->isFieldOfViewSupported;
		cameraData.bIsAspectRatioSupported = structure->isAspectRatioSupported;
		cameraData.bIsFocalLengthSupported = structure->isFocalLengthSupported;
		cameraData.bIsProjectionModeSupported = structure->isProjectionModeSupported;
		cameraData.FilmBackWidth = structure->filmBackWidth;
		cameraData.FilmBackHeight = structure->filmBackHeight;
		cameraData.bIsApertureSupported = structure->isApertureSupported;
		cameraData.bIsFocusDistanceSupported = structure->isFocusDistanceSupported;
	}
	LiveLinkProvider->UpdateSubjectStaticData(subjectName, ULiveLinkCameraRole::StaticClass(), MoveTemp(staticData));
}

void UnrealLiveLink_UpdateCameraFrame(const char *subjectName, const double worldTime,
	const UnrealLiveLink_Metadata *metadata, const UnrealLiveLink_PropertyValues *propValues, const UnrealLiveLink_Camera *frame)
{
	FLiveLinkFrameDataStruct frameData(FLiveLinkCameraFrameData::StaticStruct());
	FLiveLinkCameraFrameData& CameraData = *frameData.Cast<FLiveLinkCameraFrameData>();

	CameraData.FieldOfView = frame->fieldOfView;
	CameraData.AspectRatio = frame->aspectRatio;
	CameraData.FocalLength = frame->focalLength;
	CameraData.Aperture = frame->aperture;
	CameraData.FocusDistance = frame->focusDistance;
	CameraData.ProjectionMode = frame->isPerspective ? ELiveLinkCameraProjectionMode::Perspective : ELiveLinkCameraProjectionMode::Orthographic;

	SetFTransform(CameraData.Transform, frame->transform);

	SetBasicFrameParameters(subjectName, worldTime, metadata, propValues, frameData);

	LiveLinkProvider->UpdateSubjectFrameData(subjectName, MoveTemp(frameData));
}


void UnrealLiveLink_SetLightStructure(
	const char *subjectName, const UnrealLiveLink_Properties *properties, UnrealLiveLink_LightStatic *structure)
{
	FLiveLinkStaticDataStruct staticData(FLiveLinkLightStaticData::StaticStruct());
	FLiveLinkLightStaticData& lightData = *staticData.Cast<FLiveLinkLightStaticData>();

	if (properties)
	{
		for (int i = 0; i < properties->nameCount; i++)
		{
			lightData.PropertyNames.Add(properties->names[i]);
		}
	}
	if (structure)
	{
		lightData.bIsTemperatureSupported = structure->isTemperatureSupported;
		lightData.bIsIntensitySupported = structure->isIntensitySupported;
		lightData.bIsLightColorSupported = structure->isLightColorSupported;
		lightData.bIsInnerConeAngleSupported = structure->isInnerConeAngleSupported;
		lightData.bIsOuterConeAngleSupported = structure->isOuterConeAngleSupported;
		lightData.bIsAttenuationRadiusSupported = structure->isAttenuationRadiusSupported;
		lightData.bIsSourceLenghtSupported = structure->isSourceLengthSupported;
		lightData.bIsSourceRadiusSupported = structure->isSourceRadiusSupported;
		lightData.bIsSoftSourceRadiusSupported = structure->isSoftSourceRadiusSupported;
	}

	LiveLinkProvider->UpdateSubjectStaticData(subjectName, ULiveLinkLightRole::StaticClass(), MoveTemp(staticData));
}

void UnrealLiveLink_UpdateLightFrame(const char *subjectName, const double worldTime,
	const UnrealLiveLink_Metadata *metadata, const UnrealLiveLink_PropertyValues *propValues, const UnrealLiveLink_Light *frame)
{
	FLiveLinkFrameDataStruct frameData(FLiveLinkLightFrameData::StaticStruct());
	FLiveLinkLightFrameData& lightData = *frameData.Cast<FLiveLinkLightFrameData>();

	lightData.Temperature = frame->temperature;
	lightData.Intensity = frame->intensity;
	lightData.LightColor.R = frame->lightColor[0];
	lightData.LightColor.G = frame->lightColor[1];
	lightData.LightColor.B = frame->lightColor[2];
	lightData.LightColor.A = frame->lightColor[3];
	lightData.InnerConeAngle = frame->innerConeAngle;
	lightData.OuterConeAngle = frame->outerConeAngle;
	lightData.AttenuationRadius = frame->attenuationRadius;
	lightData.SourceRadius = frame->sourceRadius;
	lightData.SoftSourceRadius = frame->softSourceRadius;
	lightData.SourceLength = frame->sourceLength;

	SetFTransform(lightData.Transform, frame->transform);

	SetBasicFrameParameters(subjectName, worldTime, metadata, propValues, frameData);
	
	LiveLinkProvider->UpdateSubjectFrameData(subjectName, MoveTemp(frameData));
}

