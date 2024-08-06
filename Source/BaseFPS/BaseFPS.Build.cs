// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BaseFPS : ModuleRules
{
	public BaseFPS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				"BaseFPS"
			}
		);
		
		PrivateIncludePaths.AddRange(
			new string[] {
			}
		);
		
		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core", 
				"CoreUObject", 
				"Engine", 
				"Slate",
				"SlateCore",
				"UMG",
				"InputCore",
				"EnhancedInput",
				"GameplayTags",
				"CommonUser",
				"GameSubtitles",
				"RHI",
				"CommonUI",
				"CommonInput",
				"AudioModulation",
				"AudioMixer",
				"AudioMixerCore",
				"ModularGameplayActors"
			}
		);
		
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"ReplicationGraph"
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
			}
		);

		if (Target.bBuildDeveloperTools || (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Configuration != UnrealTargetConfiguration.Test))
		{
			PrivateDependencyModuleNames.Add("GameplayDebugger");
			PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER=1");
		}
		else
		{
			PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER=0");
		}
	}
}
