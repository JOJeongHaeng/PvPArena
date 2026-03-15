#include "Misc/AutomationTest.h"
#include "Player/PvPArenaCharacter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMeleeAttackTimingTest,
    "PvPArena.Combat.MeleeAttackTiming",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMeleeAttackTimingTest::RunTest(const FString& Parameters)
{
    APvPArenaCharacter* Character = NewObject<APvPArenaCharacter>();
    TestNotNull(TEXT("Character should be created"), Character);

    if (!Character)
    {
        return false;
    }

    TestFalse(TEXT("Attack should not start in progress"), Character->IsMeleeAttackInProgress());
    TestFalse(TEXT("Hit should not start triggered"), Character->HasTriggeredMeleeAttackHit());

    TestTrue(TEXT("First melee start should succeed"), Character->BeginMeleeAttack(0.0f));
    TestTrue(TEXT("Attack should enter in-progress state"), Character->IsMeleeAttackInProgress());
    TestFalse(TEXT("Starting an attack should not immediately trigger the hit"), Character->HasTriggeredMeleeAttackHit());
    TestFalse(TEXT("A second start while active should be blocked"), Character->BeginMeleeAttack(0.1f));

    Character->HandleMeleeAttackHitNotify();
    TestTrue(TEXT("Hit should be marked triggered after notify"), Character->HasTriggeredMeleeAttackHit());
    TestFalse(TEXT("Notify should not trigger twice"), Character->HandleMeleeAttackHitNotify());

    Character->FinishMeleeAttack();
    TestFalse(TEXT("Finish should clear in-progress state"), Character->IsMeleeAttackInProgress());
    TestFalse(TEXT("Finish should clear hit-triggered state"), Character->HasTriggeredMeleeAttackHit());

    return true;
}
