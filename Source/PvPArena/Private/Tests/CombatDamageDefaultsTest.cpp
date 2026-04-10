#include "Misc/AutomationTest.h"
#include "Combat/PvPCombatComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCombatDamageDefaultsTest,
    "PvPArena.Combat.DamageDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatDamageDefaultsTest::RunTest(const FString& Parameters)
{
    UPvPCombatComponent* Combat = NewObject<UPvPCombatComponent>();
    TestNotNull(TEXT("Combat component should be created"), Combat);

    if (!Combat)
    {
        return false;
    }

    TestEqual(TEXT("Default melee damage"), Combat->GetMeleeDamage(), 25.0f);
    TestEqual(TEXT("Default ranged damage"), Combat->GetRangedDamage(), 25.0f);
    TestEqual(TEXT("Default ranged cooldown"), Combat->GetRangedCooldownSeconds(), 3.0f);
    return true;
}
