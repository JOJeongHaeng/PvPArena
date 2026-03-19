#include "Misc/AutomationTest.h"
#include "Combat/PvPCombatComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRangedProjectileLaunchAlignmentTest,
    "PvPArena.Combat.RangedProjectileLaunchAlignment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRangedProjectileLaunchAlignmentTest::RunTest(const FString& Parameters)
{
    const FVector AimOrigin(100.0f, 25.0f, 80.0f);
    const FVector AimTarget(1100.0f, 225.0f, 80.0f);
    const float SpawnForwardOffset = 90.0f;

    const FVector SpawnLocation =
        UPvPCombatComponent::BuildRangedProjectileSpawnLocation(AimOrigin, AimTarget, SpawnForwardOffset);

    const FVector AimDirection = (AimTarget - AimOrigin).GetSafeNormal();
    TestEqual(
        TEXT("Projectile spawn should stay on the crosshair aim ray"),
        SpawnLocation,
        AimOrigin + (AimDirection * SpawnForwardOffset));

    return true;
}
