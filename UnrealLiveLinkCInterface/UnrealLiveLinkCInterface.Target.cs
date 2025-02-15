using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Desktop)]
public class UnrealLiveLinkCInterfaceTarget : TargetRules
{
	public UnrealLiveLinkCInterfaceTarget(TargetInfo Target) : base(Target)
	{
            LaunchModuleName = "UnrealLiveLinkCInterface";
            Type = TargetType.Program;
            bShouldCompileAsDLL = true;
            LinkType = TargetLinkType.Monolithic;

            // Lean and mean
            bBuildDeveloperTools = false;

            bBuildWithEditorOnlyData = true;
            bCompileAgainstEngine = false;
            bCompileAgainstCoreUObject = true;
            bCompileICU = false;

			IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
	}
}

