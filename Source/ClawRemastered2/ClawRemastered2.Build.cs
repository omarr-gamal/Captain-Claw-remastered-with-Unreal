// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ClawRemastered2 : ModuleRules
{
	public ClawRemastered2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Paper2D" });
	}
}
