#if WITH_EDITOR

#include "Misc/AutomationTest.h"
#include "Player/PvPArenaCharacter.h"

namespace
{
class ATestPvPArenaCharacter : public APvPArenaCharacter
{
public:
    void InvokeEndPlay()
    {
        EndPlay(EEndPlayReason::Quit);
    }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterAttackAudioCleanupTest,
    "PvPArena.Character.AttackAudioCleanup",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterAttackAudioCleanupTest::RunTest(const FString& Parameters)
{
    ATestPvPArenaCharacter* Character = NewObject<ATestPvPArenaCharacter>();
    TestNotNull(TEXT("Cleanup test should create a character"), Character);

    if (!Character)
    {
        return false;
    }

    TestNotNull(TEXT("Cleanup test should find the character melee attack component"), Character->GetMeleeAttackAudioComponent());
    TestNotNull(TEXT("Cleanup test should find the character ranged attack component"), Character->GetRangedAttackAudioComponent());

    Character->InvokeEndPlay();

    TestNull(
        TEXT("Attack audio cleanup should clear the melee attack audio reference"),
        Character->GetMeleeAttackAudioComponent());
    TestNull(
        TEXT("Attack audio cleanup should clear the ranged attack audio reference"),
        Character->GetRangedAttackAudioComponent());

    return true;
}

#endif
