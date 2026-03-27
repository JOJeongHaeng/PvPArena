#include "Misc/AutomationTest.h"
#include "UObject/Class.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDirectIpDefaultsTest,
    "PvPArena.Network.DirectIpDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectIpDefaultsTest::RunTest(const FString& Parameters)
{
    UClass* OnlineSubsystemClass = FindObject<UClass>(nullptr, TEXT("/Script/PvPArena.PvPArenaOnlineSubsystem"));
    TestNull(TEXT("Direct IP build should not register the deprecated EOS subsystem class"), OnlineSubsystemClass);
    return OnlineSubsystemClass == nullptr;
}
