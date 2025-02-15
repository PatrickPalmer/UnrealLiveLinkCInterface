
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
#include "Features/IModularFeatures.h"
#include "INetworkMessagingExtension.h"
#include "Shared/UdpMessagingSettings.h"
#include "UObject/Object.h"

//#include <cstdio>

DEFINE_LOG_CATEGORY_STATIC(LogUnrealLiveLinkCInterface, Log, All);

IMPLEMENT_APPLICATION(UnrealLiveLinkCInterface, "UnrealLiveLinkCInterface");

TSharedPtr<ILiveLinkProvider> LiveLinkProvider = nullptr;
FString LiveLinkProviderName = "";

FDelegateHandle ConnectionStatusChangedHandle{};

TArray<void(*)()> ConnectionCallbacks{};


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

void UnrealLiveLink_Initialize()
{
	GEngineLoop.PreInit(TEXT("MobuLiveLinkPlugin -Messaging"));

	// ensure target platform manager is referenced early as it must be created on the main thread
	GetTargetPlatformManager();

	ProcessNewlyLoadedUObjects();

	// Tell the module manager that it may now process newly-loaded UObjects when new C++ modules are loaded
	FModuleManager::Get().StartProcessingNewlyLoadedObjects();
	FModuleManager::Get().LoadModule(TEXT("UdpMessaging"));

	IPluginManager::Get().LoadModulesForEnabledPlugins(ELoadingPhase::PreDefault);
	IPluginManager::Get().LoadModulesForEnabledPlugins(ELoadingPhase::Default);
	IPluginManager::Get().LoadModulesForEnabledPlugins(ELoadingPhase::PostDefault);
}

int UnrealLiveLink_GetVersion()
{
	return UNREAL_LIVE_LINK_API_VERSION;
}

void UnrealLiveLink_SetProviderName(const char* ProviderName)
{
	LiveLinkProviderName = ANSI_TO_TCHAR(ProviderName);
}

int UnrealLiveLink_StartLiveLink()
{
	if (LiveLinkProviderName.IsEmpty())
	{
		LiveLinkProviderName = "C Interface";
	}

	if (LiveLinkProvider != nullptr)
	{
		UE_LOG(LogUnrealLiveLinkCInterface, Display, TEXT("Live Link C Interface already Initialized"));
		return UNREAL_LIVE_LINK_FAILED;
	}

	LiveLinkProvider = ILiveLinkProvider::CreateLiveLinkProvider(LiveLinkProviderName);
	ConnectionStatusChangedHandle = LiveLinkProvider->RegisterConnStatusChangedHandle(FLiveLinkProviderConnectionStatusChanged::FDelegate::CreateStatic(&OnConnectionStatusChanged));

	UE_LOG(LogUnrealLiveLinkCInterface, Display, TEXT("Live Link C Interface Initialized"));

	return UNREAL_LIVE_LINK_OK;
}

int UnrealLiveLink_StopLiveLink()
{
	UE_LOG(LogUnrealLiveLinkCInterface, Display, TEXT("Live Link C Interface Shutting Down"));

	if (ConnectionStatusChangedHandle.IsValid())
	{
		LiveLinkProvider->UnregisterConnStatusChangedHandle(ConnectionStatusChangedHandle);
		ConnectionStatusChangedHandle.Reset();
	}

	FTSTicker::GetCoreTicker().Tick(1.0f);

	if (LiveLinkProvider != nullptr)
	{
		UE_LOG(LogUnrealLiveLinkCInterface, Display, TEXT("LiveLinkProvider References: %d"), LiveLinkProvider.GetSharedReferenceCount());
		LiveLinkProvider = nullptr;
	}

	return UNREAL_LIVE_LINK_OK;
}

static FString GetUnicastEndpoint() 
{
	if (IModularFeatures::Get().IsModularFeatureAvailable(INetworkMessagingExtension::ModularFeatureName))
	{
		UUdpMessagingSettings* Settings = GetMutableDefault<UUdpMessagingSettings>();
		return Settings->UnicastEndpoint;
	}

	return TEXT("0.0.0.0:0");
}

void UnrealLiveLink_SetUnicastEndpoint(const char * InEndpoint)
{
	if (InEndpoint != GetUnicastEndpoint())
	{
		if (IModularFeatures::Get().IsModularFeatureAvailable(INetworkMessagingExtension::ModularFeatureName))
		{
			UnrealLiveLink_StopLiveLink();

			UUdpMessagingSettings* Settings = GetMutableDefault<UUdpMessagingSettings>();
			Settings->UnicastEndpoint = InEndpoint;
			INetworkMessagingExtension& NetworkExtension = IModularFeatures::Get().GetModularFeature<INetworkMessagingExtension>(INetworkMessagingExtension::ModularFeatureName);
			NetworkExtension.RestartServices();

			UnrealLiveLink_StartLiveLink();
		}
	}
}

int UnrealLiveLink_AddStaticEndpoint(const char * InEndpoint)
{
	if (IModularFeatures::Get().IsModularFeatureAvailable(INetworkMessagingExtension::ModularFeatureName))
	{
		INetworkMessagingExtension& NetworkExtension = IModularFeatures::Get().GetModularFeature<INetworkMessagingExtension>(INetworkMessagingExtension::ModularFeatureName);
		NetworkExtension.AddEndpoint(InEndpoint);
		return UNREAL_LIVE_LINK_OK;
	}
	return UNREAL_LIVE_LINK_FAILED;
}

int UnrealLiveLink_RemoveStaticEndpoint(const char * InEndpoint)
{
	if (IModularFeatures::Get().IsModularFeatureAvailable(INetworkMessagingExtension::ModularFeatureName))
	{
		INetworkMessagingExtension& NetworkExtension = IModularFeatures::Get().GetModularFeature<INetworkMessagingExtension>(INetworkMessagingExtension::ModularFeatureName);
		NetworkExtension.RemoveEndpoint(InEndpoint);
		return UNREAL_LIVE_LINK_OK;
	}
	return UNREAL_LIVE_LINK_FAILED;
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
		CameraData.bIsFieldOfViewSupported = CameraStructure->isFieldOfViewSupported != 0;
		CameraData.bIsAspectRatioSupported = CameraStructure->isAspectRatioSupported != 0;
		CameraData.bIsFocalLengthSupported = CameraStructure->isFocalLengthSupported != 0;
		CameraData.bIsProjectionModeSupported = CameraStructure->isProjectionModeSupported != 0;
		CameraData.FilmBackWidth = CameraStructure->filmBackWidth;
		CameraData.FilmBackHeight = CameraStructure->filmBackHeight;
		CameraData.bIsApertureSupported = CameraStructure->isApertureSupported != 0;
		CameraData.bIsFocusDistanceSupported = CameraStructure->isFocusDistanceSupported != 0;
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
		LightData.bIsTemperatureSupported = LightStructure->isTemperatureSupported != 0;
		LightData.bIsIntensitySupported = LightStructure->isIntensitySupported != 0;
		LightData.bIsLightColorSupported = LightStructure->isLightColorSupported != 0;
		LightData.bIsInnerConeAngleSupported = LightStructure->isInnerConeAngleSupported != 0;
		LightData.bIsOuterConeAngleSupported = LightStructure->isOuterConeAngleSupported != 0;
		LightData.bIsAttenuationRadiusSupported = LightStructure->isAttenuationRadiusSupported != 0;
		LightData.bIsSourceLenghtSupported = LightStructure->isSourceLengthSupported != 0;
		LightData.bIsSourceRadiusSupported = LightStructure->isSourceRadiusSupported != 0;
		LightData.bIsSoftSourceRadiusSupported = LightStructure->isSoftSourceRadiusSupported != 0;
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

