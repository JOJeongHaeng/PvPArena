#include "Misc/AutomationTest.h"
#include "Player/PvPArenaCharacter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterHealthDefaultsTest,
    "PvPArena.Character.HealthDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterHealthDefaultsTest::RunTest(const FString& Parameters)
{
    APvPArenaCharacter* Character = NewObject<APvPArenaCharacter>();
    TestNotNull(TEXT("Character should be created"), Character);

    if (!Character)
    {
        return false;
    }

    TestEqual(TEXT("Default max health"), Character->GetMaxHealth(), 100.0f);
    TestEqual(TEXT("Default current health"), Character->GetCurrentHealth(), 100.0f);
    return true;
}
