// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class minijam184 : ModuleRules
{
	public minijam184(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput" });
	}
}
