#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPvPArenaModuleLoadTest,
    "PvPArena.Core.ModuleLoad",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPvPArenaModuleLoadTest::RunTest(const FString& Parameters)
{
    const bool bLoaded = FModuleManager::Get().IsModuleLoaded(TEXT("PvPArena"));
    TestTrue(TEXT("PvPArena module should be loaded"), bLoaded);
    return true;
}
