#include "Misc/AutomationTest.h"
#include "Combat/PvPCombatComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCombatCooldownTest,
    "PvPArena.Combat.CooldownValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatCooldownTest::RunTest(const FString& Parameters)
{
    UPvPCombatComponent* Combat = NewObject<UPvPCombatComponent>();
    TestNotNull(TEXT("Combat component should be created"), Combat);

    if (!Combat)
    {
        return false;
    }

    TestTrue(TEXT("First melee is allowed"), Combat->CanUseMelee(0.0f));
    Combat->MarkMeleeUsed(0.0f);
    TestFalse(TEXT("Immediate melee reuse is blocked"), Combat->CanUseMelee(0.1f));
    return true;
}
