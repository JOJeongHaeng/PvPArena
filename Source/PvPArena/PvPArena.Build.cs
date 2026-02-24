using UnrealBuildTool;

public class PvPArena : ModuleRules
{
    public PvPArena(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "UMG"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "NetCore",
            "Slate",
            "SlateCore"
        });
    }
}
