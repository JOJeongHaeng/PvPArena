#include "Misc/AutomationTest.h"
#include "Combat/PvPCombatComponent.h"
#include "UObject/UnrealType.h"

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

    const FFloatProperty* RangedCooldownProperty = FindFProperty<FFloatProperty>(UPvPCombatComponent::StaticClass(), TEXT("RangedCooldownSeconds"));
    TestNotNull(TEXT("Ranged cooldown property should exist"), RangedCooldownProperty);
    TestEqual(
        TEXT("Ranged cooldown should be tuned for a longer 5 second reuse window"),
        RangedCooldownProperty ? RangedCooldownProperty->GetPropertyValue_InContainer(Combat) : 0.0f,
        5.0f);

    const FFloatProperty* NextAllowedRangedTimeProperty = FindFProperty<FFloatProperty>(UPvPCombatComponent::StaticClass(), TEXT("NextAllowedRangedTime"));
    TestNotNull(TEXT("Ranged cooldown end time property should exist"), NextAllowedRangedTimeProperty);
    TestTrue(
        TEXT("Ranged cooldown end time should replicate so client HUD can count down"),
        NextAllowedRangedTimeProperty && NextAllowedRangedTimeProperty->HasAnyPropertyFlags(CPF_Net));

    TestTrue(TEXT("First ranged attack is allowed"), Combat->CanUseRanged(0.0f));
    Combat->MarkRangedUsed(0.0f);
    TestFalse(TEXT("Immediate ranged reuse is blocked"), Combat->CanUseRanged(0.1f));
    return true;
}
