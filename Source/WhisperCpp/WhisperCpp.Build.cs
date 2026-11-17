// Georgy Treshchev 2024.

using UnrealBuildTool;
using System.IO;

// Compiles upstream whisper.cpp + ggml (Source/ThirdParty) through one wrapper TU per upstream file (Private/RSR_*.c/.cpp)
public class WhisperCpp : ModuleRules
{
	public WhisperCpp(ReadOnlyTargetRules Target) : base(Target)
	{
		// Upstream code must not see UE's PCH, and its static symbols must not be merged into unity blobs
		PCHUsage = PCHUsageMode.NoPCHs;
		bUseUnity = false;
		bEnableExceptions = true;
		CStandard = CStandardVersion.C17;
		// Upstream relies on undefined macros evaluating to 0 in #if
		CppCompileWarningSettings.UndefinedIdentifierWarningLevel = WarningLevel.Off;

		if (!Target.IsInPlatformGroup(UnrealPlatformGroup.Apple) && Target.Architecture == UnrealArch.X64)
		{
			// AVX2 (+FMA/F16C) is the main CPU speed-up for ggml. shortcut: CPUs without AVX2 (pre-2013) will crash, switch to AVX if you must support them
			MinCpuArchX64 = MinimumCpuArchitectureX64.AVX2;
		}

		PrivateDependencyModuleNames.Add("Core");

		string ThirdPartyDir = Path.Combine(ModuleDirectory, "..", "ThirdParty");
		PublicIncludePaths.AddRange(new string[]
		{
			Path.Combine(ThirdPartyDir, "whisper.cpp", "include"),
			Path.Combine(ThirdPartyDir, "ggml", "include"),
		});
		PrivateIncludePaths.AddRange(new string[]
		{
			Path.Combine(ModuleDirectory, "Private"),
			Path.Combine(ThirdPartyDir, "whisper.cpp", "src"),
			Path.Combine(ThirdPartyDir, "ggml", "src"),
			Path.Combine(ThirdPartyDir, "ggml", "src", "ggml-cpu"),
		});

		// Export the C API from the module DLL in modular (editor) builds
		if (Target.LinkType == TargetLinkType.Modular)
		{
			PublicDefinitions.AddRange(new string[] { "WHISPER_SHARED", "GGML_SHARED", "GGML_BACKEND_SHARED" });
			PrivateDefinitions.AddRange(new string[] { "WHISPER_BUILD", "GGML_BUILD", "GGML_BACKEND_BUILD" });
		}

		PrivateDefinitions.AddRange(new string[]
		{
			"GGML_USE_CPU",
			"GGML_USE_CPU_REPACK",
			"GGML_USE_LLAMAFILE",
			"WHISPER_VERSION=\"1.9.5\"",
		});

		// Vulkan GPU acceleration. Enabled automatically once Scripts/GenerateVulkanShaders.ps1 has written Private/VulkanShaders.
		// The Vulkan loader (vulkan-1) is resolved at runtime by the dynamic dispatcher, so there is no link dependency
		bool bHasVulkanShaders = File.Exists(Path.Combine(ModuleDirectory, "Private", "VulkanShaders", "ggml-vulkan-shaders.hpp"));
		bool bVulkanPlatform = Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Linux;
		if (bHasVulkanShaders && bVulkanPlatform)
		{
			string EngineDir = Path.GetFullPath(Target.RelativeEnginePath);
			PrivateIncludePaths.Add(Path.Combine(EngineDir, "Source", "ThirdParty", "Vulkan", "Include"));
			PrivateIncludePaths.Add(Path.Combine(ThirdPartyDir, "ggml", "src", "ggml-vulkan"));
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "VulkanShaders"));
			PrivateDefinitions.Add("GGML_USE_VULKAN");
			PrivateDefinitions.Add("VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1");
			string DefinesFile = Path.Combine(ModuleDirectory, "Private", "VulkanShaders", "Defines.txt");
			if (File.Exists(DefinesFile))
			{
				foreach (string Define in File.ReadAllLines(DefinesFile))
				{
					if (Define.Trim().Length > 0)
					{
						PrivateDefinitions.Add(Define.Trim());
					}
				}
			}
		}
	}
}
