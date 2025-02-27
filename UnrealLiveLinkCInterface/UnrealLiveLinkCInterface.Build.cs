using UnrealBuildTool;
using System.IO;

public class UnrealLiveLinkCInterface : ModuleRules
{
	public UnrealLiveLinkCInterface(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateIncludePaths.AddRange( new string[] { "Runtime/Launch/Public", "Runtime/Launch/Private" }  );

		// Unreal dependency modules
		PrivateDependencyModuleNames.AddRange( new string[] 
		{
			"Core",
			"CoreUObject",
			"ApplicationCore",
			"Projects",
			"UdpMessaging",
			"LiveLinkInterface",
			"LiveLinkMessageBusFramework",
		} );
	}
}

