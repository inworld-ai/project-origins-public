// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class InWorldRT : ModuleRules
{
	public InWorldRT(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"InworldAIIntegration",
			"InworldAIClient" ,
			"AudioMixer",
			"AudioCaptureCore"
		});

		PrivateDependencyModuleNames.Add("AudioCaptureRtAudio");

		PrivateDefinitions.Add("INWORLD_OVR_LIPSYNC=1");
	}
}
