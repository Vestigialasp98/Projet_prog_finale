// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjetProgFinal : ModuleRules
{
	public ProjetProgFinal(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput"
		});
		
		PrivateDependencyModuleNames.AddRange(new string[] { 
			"AIModule",
			"GameplayTasks", // <-- AJOUTÉ POUR L'IA
			"NavigationSystem", // <-- AJOUTÉ POUR L'IA
			"Niagara", 
			"UMG" 
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include Public/Private C++ files
		// PrivateIncludePaths.Add("MyProject/Private");
		// PublicIncludePaths.Add("MyProject/Public");
	}
}
