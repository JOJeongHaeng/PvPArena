#include "Misc/AutomationTest.h"
#include "Player/PvPArenaCharacter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterDeathFlowTest,
    "PvPArena.Character.DeathFlow",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterDeathFlowTest::RunTest(const FString& Parameters)
{
    APvPArenaCharacter* Character = NewObject<APvPArenaCharacter>();
    TestNotNull(TEXT("Character should be created"), Character);

    if (!Character)
    {
        return false;
    }

    Character->ApplyServerDamage(150.0f, nullptr);
    TestTrue(TEXT("Character should be dead after lethal damage"), Character->IsDead());
    return true;
}
