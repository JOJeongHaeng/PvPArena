#include "Misc/AutomationTest.h"
#include "Combat/PvPCombatComponent.h"
#include "Combat/PvPProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRangedProjectileDefaultsTest,
    "PvPArena.Combat.RangedProjectileDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRangedProjectileDefaultsTest::RunTest(const FString& Parameters)
{
    const FBoolProperty* DebugEnabledProperty = FindFProperty<FBoolProperty>(APvPProjectile::StaticClass(), TEXT("bDrawProjectileDebug"));
    const FFloatProperty* DebugDurationProperty = FindFProperty<FFloatProperty>(APvPProjectile::StaticClass(), TEXT("ProjectileDebugDrawTime"));
    const FFloatProperty* DebugThicknessProperty = FindFProperty<FFloatProperty>(APvPProjectile::StaticClass(), TEXT("ProjectileDebugLineThickness"));
    const FClassProperty* ProjectileClassProperty = FindFProperty<FClassProperty>(UPvPCombatComponent::StaticClass(), TEXT("RangedProjectileClass"));
    TestNotNull(TEXT("Projectile should expose a draw-debug toggle"), DebugEnabledProperty);
    TestNotNull(TEXT("Projectile should expose a debug draw duration"), DebugDurationProperty);
    TestNotNull(TEXT("Projectile should expose a debug line thickness"), DebugThicknessProperty);
    TestNotNull(TEXT("Combat component should expose a ranged projectile class"), ProjectileClassProperty);

    if (!DebugEnabledProperty || !DebugDurationProperty || !DebugThicknessProperty || !ProjectileClassProperty)
    {
        return false;
    }

    const UPvPCombatComponent* Combat = GetDefault<UPvPCombatComponent>();
    TestNotNull(TEXT("Combat CDO should exist"), Combat);

    UClass* ProjectileClass = Combat
        ? Cast<UClass>(ProjectileClassProperty->GetPropertyValue_InContainer(Combat))
        : nullptr;
    TestNotNull(TEXT("Combat should default to a ranged projectile class"), ProjectileClass);

    if (!ProjectileClass)
    {
        return false;
    }

    APvPProjectile* Projectile = NewObject<APvPProjectile>(GetTransientPackage(), ProjectileClass);
    TestNotNull(TEXT("Projectile should be constructible"), Projectile);

    UProjectileMovementComponent* ProjectileMovement = Projectile ? Projectile->FindComponentByClass<UProjectileMovementComponent>() : nullptr;
    TestNotNull(TEXT("Projectile should have a projectile movement component"), ProjectileMovement);

    TestNull(TEXT("Projectile should no longer create a Niagara effect component"), Projectile ? Projectile->FindComponentByClass<UNiagaraComponent>() : nullptr);

    TestTrue(TEXT("Projectile should ignore gravity for a straight magic bolt"), ProjectileMovement && ProjectileMovement->ProjectileGravityScale == 0.0f);
    TestEqual(TEXT("Projectile should slow down slightly for easier visual tracking"), ProjectileMovement ? ProjectileMovement->InitialSpeed : 0.0f, 1800.0f);
    TestEqual(TEXT("Projectile max speed should match the lowered initial speed"), ProjectileMovement ? ProjectileMovement->MaxSpeed : 0.0f, 1800.0f);
    TestTrue(TEXT("Projectile draw-debug should default on for alignment checks"), DebugEnabledProperty->GetPropertyValue_InContainer(Projectile));
    TestEqual(TEXT("Projectile should keep its debug trail visible briefly"), DebugDurationProperty->GetPropertyValue_InContainer(Projectile), 1.0f);
    TestEqual(TEXT("Projectile should use a readable debug trail thickness"), DebugThicknessProperty->GetPropertyValue_InContainer(Projectile), 1.5f);

    return true;
}
