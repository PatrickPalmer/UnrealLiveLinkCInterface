
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

//#include <cstdio>

DEFINE_LOG_CATEGORY_STATIC(LogUnrealLiveLinkCInterface, Log, All);

IMPLEMENT_APPLICATION(UnrealLiveLinkCInterface, "UnrealLiveLinkCInterface");

class FLiveLinkStreamedSubjectManager;

TSharedPtr<ILiveLinkProvider> LiveLinkProvider;
TSharedPtr<FLiveLinkStreamedSubjectManager> LiveLinkStreamManager;
FDelegateHandle ConnectionStatusChangedHandle;

TArray<void(*)()> ConnectionCallbacks;

int32_t TimecodeRates[UNREAL_LIVE_LINK_TIMECODE_120 + 1][2] = {
		{ 0, 0 },		// unknown
		{ 24000, 1001 },	// 23.98
		{ 24, 1 },		// 24
		{ 25, 1 },		// 25
		{ 30000, 1001 },	// 29.97 NDF
		{ 30000, 1001 },	// 29.97 DF
		{ 30, 1 },		// 30
		{ 48000, 1001 },	// 47.95
		{ 48, 1 },		// 48
		{ 50, 1 },		// 50
		{ 60000, 1001 },	// 59.94 NDF
		{ 60000, 1001 },	// 59.94 DF
		{ 60, 1 },		// 60
		{ 72, 1 },		// 72
		{ 96, 1 },		// 96
		{ 100, 1 },		// 100
		{ 120, 1 }		// 120
};


// set FTransform from Unreal Live Link C Interface Transform
static void SetFTransform(FTransform &Transform, const UnrealLiveLink_Transform &InTransform)
{
	Transform.SetRotation(FQuat(InTransform.rotation[0], InTransform.rotation[1], InTransform.rotation[2], InTransform.rotation[3]));
	Transform.SetTranslation(FVector(InTransform.translation[0], InTransform.translation[1], InTransform.translation[2]));
	Transform.SetScale3D(FVector(InTransform.scale[0], InTransform.scale[1], InTransform.scale[2]));
}


static void OnConnectionStatusChanged()
{
	for (const TArray<void (*)()>::ElementType &Callback : ConnectionCallbacks)
	{
		Callback();
	}
}

int UnrealLiveLink_GetVersion()
{
	return UNREAL_LIVE_LINK_API_VERSION;
}

int UnrealLiveLink_InitializeMessagingInterface(const char *InterfaceName)
{
	// SET UP
	GEngineLoop.PreInit(TEXT("UnrealLiveLinkCInterface -Messaging"));
	ProcessNewlyLoadedUObjects();

	// Tell the module manager now process newly-loaded UObjects when new C++ modules are loaded
	FModuleManager::Get().StartProcessingNewlyLoadedObjects();

	FModuleManager::Get().LoadModule(TEXT("UdpMessaging"));

	GLog->TearDown();

	LiveLinkProvider = ILiveLinkProvider::CreateLiveLinkProvider(ANSI_TO_TCHAR(InterfaceName));
	ConnectionStatusChangedHandle = LiveLinkProvider->RegisterConnStatusChangedHandle(FLiveLinkProviderConnectionStatusChanged::FDelegate::CreateStatic(&OnConnectionStatusChanged));

	// We do not tick the core engine but we need to tick the ticker to make sure the message bus endpoint in LiveLinkProvider is
	// up to date
	FTicker::GetCoreTicker().Tick(1.f);

#ifdef TODO
	LiveLinkStreamManager = MakeShareable(new FLiveLinkStreamedSubjectManager());
#endif

	UE_LOG(LogUnrealLiveLinkCInterface, Display, TEXT("Live Link C Interface Initialized"));

	return UNREAL_LIVE_LINK_OK;
}

int UnrealLiveLink_UninitializeMessagingInterface()
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

	return UNREAL_LIVE_LINK_OK;
}

void UnrealLiveLink_RegisterConnectionUpdateCallback(void (*Callback)())
{
	ConnectionCallbacks.Push(Callback);
}

int UnrealLiveLink_HasConnection()
{
	return LiveLinkProvider->HasConnection() ? UNREAL_LIVE_LINK_OK : UNREAL_LIVE_LINK_NOT_CONNECTED;
}


static void SetBasicFrameParameters(const char *SubjectName, const double WorldTime,
	const UnrealLiveLink_Metadata *Metadata, const UnrealLiveLink_PropertyValues *PropValues, FLiveLinkFrameDataStruct &FrameData)
{
	FLiveLinkBaseFrameData& BaseData = *FrameData.Cast<FLiveLinkBaseFrameData>();

	BaseData.WorldTime = WorldTime;
	
	if (PropValues)
	{
		for (int Idx = 0; Idx < PropValues->valueCount; Idx++)
		{
			BaseData.PropertyValues.Add(PropValues->values[Idx]);
		}
	}

	if (Metadata)
	{
		for (int Idx = 0; Idx < Metadata->keyValueCount; Idx++)
		{
			BaseData.MetaData.StringMetaData[Metadata->keyValues[Idx].name] = Metadata->keyValues[Idx].value;
		}

		// set sceneTime
		bool DropFrame = Metadata->timecode.format == UNREAL_LIVE_LINK_TIMECODE_29_97_DF || Metadata->timecode.format == UNREAL_LIVE_LINK_TIMECODE_59_94_DF;
		FTimecode Timecode(Metadata->timecode.hours, Metadata->timecode.minutes, Metadata->timecode.seconds, Metadata->timecode.frames, DropFrame);

		int32_t Nom = TimecodeRates[Metadata->timecode.format][0];
		int32_t Denom = TimecodeRates[Metadata->timecode.format][1];

		FQualifiedFrameTime FrameTime(Timecode, FFrameRate(Nom, Denom));
		BaseData.MetaData.SceneTime = FrameTime;
	}
}


void UnrealLiveLink_SetBasicStructure(const char *SubjectName, const UnrealLiveLink_Properties *Properties)
{
	FLiveLinkStaticDataStruct StaticData(FLiveLinkBaseStaticData::StaticStruct());
	FLiveLinkBaseStaticData& BaseData = *StaticData.Cast<FLiveLinkBaseStaticData>();

	if (Properties)
	{
		for (int Idx = 0; Idx < Properties->nameCount; Idx++)
		{
			BaseData.PropertyNames.Add(Properties->names[Idx]);
		}
	}

	LiveLinkProvider->UpdateSubjectStaticData(SubjectName, ULiveLinkBasicRole::StaticClass(), MoveTemp(StaticData));
}

void UnrealLiveLink_UpdateBasicFrame(const char *SubjectName, const double WorldTime,
	const UnrealLiveLink_Metadata *Metadata, const UnrealLiveLink_PropertyValues *PropValues)
{
	FLiveLinkFrameDataStruct FrameData(FLiveLinkBaseFrameData::StaticStruct());

	SetBasicFrameParameters(SubjectName, WorldTime, Metadata, PropValues, FrameData);

	LiveLinkProvider->UpdateSubjectFrameData(SubjectName, MoveTemp(FrameData));
}


void UnrealLiveLink_SetAnimationStructure(
	const char *SubjectName, const UnrealLiveLink_Properties *Properties, UnrealLiveLink_AnimationStatic *AnimStructure)
{
	FLiveLinkStaticDataStruct StaticData(FLiveLinkSkeletonStaticData::StaticStruct());
	FLiveLinkSkeletonStaticData& AnimData = *StaticData.Cast<FLiveLinkSkeletonStaticData>();

	if (Properties)
	{
		for (int Idx = 0; Idx < Properties->nameCount; Idx++)
		{
			AnimData.PropertyNames.Add(Properties->names[Idx]);
		}
	}

	TArray<FName> Names;
	TArray<int32> Indices;
	for (int Idx = 0; Idx < AnimStructure->boneCount; Idx++)
	{
		Names.Add(AnimStructure->bones[Idx].name);
		Indices.Add(AnimStructure->bones[Idx].parentIndex);
	}

	AnimData.SetBoneNames(Names);
	AnimData.SetBoneParents(Indices);

	LiveLinkProvider->UpdateSubjectStaticData(SubjectName, ULiveLinkAnimationRole::StaticClass(), MoveTemp(StaticData));
}

void UnrealLiveLink_UpdateAnimationFrame(const char *SubjectName, const double WorldTime,
	const UnrealLiveLink_Metadata *Metadata, const UnrealLiveLink_PropertyValues *PropValues,
	const UnrealLiveLink_Animation *Frame)
{
	FLiveLinkFrameDataStruct FrameData(FLiveLinkAnimationFrameData::StaticStruct());
	FLiveLinkAnimationFrameData& AnimData = *FrameData.Cast<FLiveLinkAnimationFrameData>();

	for (int Idx = 0; Idx < Frame->transformCount; Idx++)
	{
		UnrealLiveLink_Transform &Transform = Frame->transforms[Idx];

		FTransform UETrans;

		UETrans.SetRotation(FQuat(Transform.rotation[0], Transform.rotation[1], Transform.rotation[2], Transform.rotation[3]));

		UETrans.SetTranslation(FVector(Transform.translation[0], Transform.translation[1], Transform.translation[2]));

		UETrans.SetScale3D(FVector(Transform.scale[0], Transform.scale[1], Transform.scale[2]));

		AnimData.Transforms.Add(UETrans);
	}

	SetBasicFrameParameters(SubjectName, WorldTime, Metadata, PropValues, FrameData);
	
	LiveLinkProvider->UpdateSubjectFrameData(SubjectName, MoveTemp(FrameData));
}


void UnrealLiveLink_SetTransformStructure(const char *SubjectName, const UnrealLiveLink_Properties *Properties)
{
	FLiveLinkStaticDataStruct StaticData(FLiveLinkTransformStaticData::StaticStruct());
	FLiveLinkTransformStaticData& XformData = *StaticData.Cast<FLiveLinkTransformStaticData>();

	if (Properties)
	{
		for (int Idx = 0; Idx < Properties->nameCount; Idx++)
		{
			XformData.PropertyNames.Add(Properties->names[Idx]);
		}
	}

	LiveLinkProvider->UpdateSubjectStaticData(SubjectName, ULiveLinkTransformRole::StaticClass(), MoveTemp(StaticData));
}

void UnrealLiveLink_UpdateTransformFrame(const char *SubjectName, const double WorldTime,
	const UnrealLiveLink_Metadata *Metadata, const UnrealLiveLink_PropertyValues *PropValues,
	const UnrealLiveLink_Transform *Frame)
{
	FLiveLinkFrameDataStruct FrameData(FLiveLinkTransformFrameData::StaticStruct());
	FLiveLinkTransformFrameData& XformData = *FrameData.Cast<FLiveLinkTransformFrameData>();

	SetFTransform(XformData.Transform, *Frame);

	SetBasicFrameParameters(SubjectName, WorldTime, Metadata, PropValues, FrameData);
	
	LiveLinkProvider->UpdateSubjectFrameData(SubjectName, MoveTemp(FrameData));
}


void UnrealLiveLink_SetCameraStructure(
	const char *SubjectName, const UnrealLiveLink_Properties *Properties, UnrealLiveLink_CameraStatic *CameraStructure)
{
	FLiveLinkStaticDataStruct StaticData(FLiveLinkCameraStaticData::StaticStruct());
	FLiveLinkCameraStaticData& CameraData = *StaticData.Cast<FLiveLinkCameraStaticData>();

	if (Properties)
	{
		for (int Idx = 0; Idx < Properties->nameCount; Idx++)
		{
			CameraData.PropertyNames.Add(Properties->names[Idx]);
		}
	}
	if (CameraStructure)
	{
		CameraData.bIsFieldOfViewSupported = CameraStructure->isFieldOfViewSupported;
		CameraData.bIsAspectRatioSupported = CameraStructure->isAspectRatioSupported;
		CameraData.bIsFocalLengthSupported = CameraStructure->isFocalLengthSupported;
		CameraData.bIsProjectionModeSupported = CameraStructure->isProjectionModeSupported;
		CameraData.FilmBackWidth = CameraStructure->filmBackWidth;
		CameraData.FilmBackHeight = CameraStructure->filmBackHeight;
		CameraData.bIsApertureSupported = CameraStructure->isApertureSupported;
		CameraData.bIsFocusDistanceSupported = CameraStructure->isFocusDistanceSupported;
	}
	LiveLinkProvider->UpdateSubjectStaticData(SubjectName, ULiveLinkCameraRole::StaticClass(), MoveTemp(StaticData));
}

void UnrealLiveLink_UpdateCameraFrame(const char *SubjectName, const double WorldTime,
	const UnrealLiveLink_Metadata *Metadata, const UnrealLiveLink_PropertyValues *PropValues, const UnrealLiveLink_Camera *Frame)
{
	FLiveLinkFrameDataStruct FrameData(FLiveLinkCameraFrameData::StaticStruct());
	FLiveLinkCameraFrameData& CameraData = *FrameData.Cast<FLiveLinkCameraFrameData>();

	CameraData.FieldOfView = Frame->fieldOfView;
	CameraData.AspectRatio = Frame->aspectRatio;
	CameraData.FocalLength = Frame->focalLength;
	CameraData.Aperture = Frame->aperture;
	CameraData.FocusDistance = Frame->focusDistance;
	CameraData.ProjectionMode = Frame->isPerspective ? ELiveLinkCameraProjectionMode::Perspective : ELiveLinkCameraProjectionMode::Orthographic;

	SetFTransform(CameraData.Transform, Frame->transform);

	SetBasicFrameParameters(SubjectName, WorldTime, Metadata, PropValues, FrameData);

	LiveLinkProvider->UpdateSubjectFrameData(SubjectName, MoveTemp(FrameData));
}


void UnrealLiveLink_SetLightStructure(
	const char *SubjectName, const UnrealLiveLink_Properties *Properties, UnrealLiveLink_LightStatic *LightStructure)
{
	FLiveLinkStaticDataStruct StaticData(FLiveLinkLightStaticData::StaticStruct());
	FLiveLinkLightStaticData& LightData = *StaticData.Cast<FLiveLinkLightStaticData>();

	if (Properties)
	{
		for (int Idx = 0; Idx < Properties->nameCount; Idx++)
		{
			LightData.PropertyNames.Add(Properties->names[Idx]);
		}
	}
	if (LightStructure)
	{
		LightData.bIsTemperatureSupported = LightStructure->isTemperatureSupported;
		LightData.bIsIntensitySupported = LightStructure->isIntensitySupported;
		LightData.bIsLightColorSupported = LightStructure->isLightColorSupported;
		LightData.bIsInnerConeAngleSupported = LightStructure->isInnerConeAngleSupported;
		LightData.bIsOuterConeAngleSupported = LightStructure->isOuterConeAngleSupported;
		LightData.bIsAttenuationRadiusSupported = LightStructure->isAttenuationRadiusSupported;
		LightData.bIsSourceLenghtSupported = LightStructure->isSourceLengthSupported;
		LightData.bIsSourceRadiusSupported = LightStructure->isSourceRadiusSupported;
		LightData.bIsSoftSourceRadiusSupported = LightStructure->isSoftSourceRadiusSupported;
	}

	LiveLinkProvider->UpdateSubjectStaticData(SubjectName, ULiveLinkLightRole::StaticClass(), MoveTemp(StaticData));
}

void UnrealLiveLink_UpdateLightFrame(const char *SubjectName, const double WorldTime,
	const UnrealLiveLink_Metadata *Metadata, const UnrealLiveLink_PropertyValues *PropValues, const UnrealLiveLink_Light *Frame)
{
	FLiveLinkFrameDataStruct FrameData(FLiveLinkLightFrameData::StaticStruct());
	FLiveLinkLightFrameData& LightData = *FrameData.Cast<FLiveLinkLightFrameData>();

	LightData.Temperature = Frame->temperature;
	LightData.Intensity = Frame->intensity;
	LightData.LightColor.R = Frame->lightColor[0];
	LightData.LightColor.G = Frame->lightColor[1];
	LightData.LightColor.B = Frame->lightColor[2];
	LightData.LightColor.A = 255;
	LightData.InnerConeAngle = Frame->innerConeAngle;
	LightData.OuterConeAngle = Frame->outerConeAngle;
	LightData.AttenuationRadius = Frame->attenuationRadius;
	LightData.SourceRadius = Frame->sourceRadius;
	LightData.SoftSourceRadius = Frame->softSourceRadius;
	LightData.SourceLength = Frame->sourceLength;

	SetFTransform(LightData.Transform, Frame->transform);

	SetBasicFrameParameters(SubjectName, WorldTime, Metadata, PropValues, FrameData);
	
	LiveLinkProvider->UpdateSubjectFrameData(SubjectName, MoveTemp(FrameData));
}

