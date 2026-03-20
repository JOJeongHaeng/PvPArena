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
    const FVector CharacterLocation(260.0f, 57.0f, 80.0f);
    const float SpawnForwardOffset = 90.0f;

    const FVector SpawnLocation =
        UPvPCombatComponent::BuildRangedProjectileSpawnLocation(AimOrigin, AimTarget, CharacterLocation, SpawnForwardOffset);

    const FVector AimDirection = (AimTarget - AimOrigin).GetSafeNormal();
    const float CharacterProjectionDistance = FVector::DotProduct(CharacterLocation - AimOrigin, AimDirection);
    TestEqual(
        TEXT("Projectile spawn should stay on the crosshair aim ray"),
        SpawnLocation,
        AimOrigin + (AimDirection * (CharacterProjectionDistance + SpawnForwardOffset)));
    TestTrue(
        TEXT("Projectile spawn should stay in front of the character anchor point on the aim ray"),
        FVector::DotProduct(SpawnLocation - CharacterLocation, AimDirection) >= SpawnForwardOffset - KINDA_SMALL_NUMBER);

    return true;
}
