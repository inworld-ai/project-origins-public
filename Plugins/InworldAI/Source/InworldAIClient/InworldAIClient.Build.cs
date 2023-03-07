// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class InworldAIClient : ModuleRules
{
    private string SentryThirdPartyDirectory
    {
        get
        {
            return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../Sentry/Source/ThirdParty/"));
        }
    }
    
    private string ThirdPartyDirectory
    {
        get
        {
            return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/"));
        }
    }

    private string ThirdPartyLibrariesDirectory
    {
        get
        {
            if(Target.Platform == UnrealTargetPlatform.Win64)
            {
                return Path.Combine(ThirdPartyDirectory, "Libraries/Win64");
            }
            else if(Target.Platform == UnrealTargetPlatform.Mac)
            {
                return Path.Combine(ThirdPartyDirectory, "Libraries/Mac");
            }
            else
            {
                return Path.Combine(ThirdPartyDirectory, "Libraries/Unknown");
            }
        }
    }

    public InworldAIClient(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrecompileForTargets = PrecompileTargetsType.Any;
        bPrecompile = true;

        PublicDependencyModuleNames.AddRange(
            new string[] 
            { 
                "Core", 
                "CoreUObject", 
                "Engine", 
                "InputCore",
                "HTTP",
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Sockets",
                "OpenSSL",
                "Projects",
                "Json",
                "JsonUtilities",
            }
            );

        //PrivateDependencyModuleNames.Add("Sentry");
        //PrivateDefinitions.Add("INWORLD_SENTRY=1");

        PublicIncludePaths.Add(Path.Combine(SentryThirdPartyDirectory, "Win64/include"));

        PublicDefinitions.Add("GOOGLE_PROTOBUF_NO_RTTI");
        PublicDefinitions.Add("GPR_FORBID_UNREACHABLE_CODE");
        PublicDefinitions.Add("GRPC_ALLOW_EXCEPTIONS=0");
        
        PrivateDefinitions.Add("INWORLD_ONSCREEN_LOG=0");
        PrivateDefinitions.Add("INWORLD_ONSCREEN_LOG_ERROR=0");

        PublicIncludePaths.Add(Path.Combine(ThirdPartyDirectory, "Includes"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/proto"));

        if(Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "address_sorting.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "cares.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "gpr.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "grpc.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "grpc++.lib"));
            //PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "grpc_unsecure.lib"));
            //PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "grpc++_unsecure.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libprotobuf.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "upb.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_base.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_malloc_internal.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_raw_logging_internal.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_spinlock_wait.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_throw_delegate.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_time.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_time_zone.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_graphcycles_internal.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_synchronization.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_cord.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_str_format_internal.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_strings.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_strings_internal.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_status.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_statusor.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_bad_optional_access.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_stacktrace.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_symbolize.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "absl_int128.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "re2.lib"));
        }
        else if(Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libaddress_sorting.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libcares.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libgpr.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libgrpc.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libgrpc++.a"));
            //PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libgrpc_unsecure.a"));
            //PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libgrpc++_unsecure.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libprotobuf.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libupb.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_base.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_malloc_internal.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_raw_logging_internal.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_spinlock_wait.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_throw_delegate.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_time.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_time_zone.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_graphcycles_internal.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_synchronization.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_cord.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_str_format_internal.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_strings.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_strings_internal.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_status.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_statusor.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_bad_optional_access.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_stacktrace.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_symbolize.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libabsl_int128.a"));
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibrariesDirectory, "libre2.a"));
        }

        AddEngineThirdPartyPrivateStaticDependencies(Target, "zlib");
    }
}
