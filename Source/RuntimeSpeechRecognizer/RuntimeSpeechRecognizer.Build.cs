// Georgy Treshchev 2024.

using UnrealBuildTool;

public class RuntimeSpeechRecognizer : ModuleRules
{
	public RuntimeSpeechRecognizer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Core",
				"SignalProcessing",
				"AudioPlatformConfiguration",
				"AudioCaptureCore",
				"WhisperCpp"
			}
		);

		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
					"Slate",
					"SlateCore",
					"DeveloperToolSettings"
				});
		}

	}
}
