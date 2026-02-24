#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPvPArenaModuleBootstrapTest,
    "PvPArena.Bootstrap.ModuleLoads",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPvPArenaModuleBootstrapTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("PvPArena module should be loaded"), FModuleManager::Get().IsModuleLoaded(TEXT("PvPArena")));
    return true;
}
