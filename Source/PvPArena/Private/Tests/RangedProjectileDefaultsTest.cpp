#include "Misc/AutomationTest.h"
#include "Combat/PvPCombatComponent.h"
#include "Combat/PvPProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRangedProjectileDefaultsTest,
    "PvPArena.Combat.RangedProjectileDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRangedProjectileDefaultsTest::RunTest(const FString& Parameters)
{
    const FString ExpectedProjectileEffectPath =
        TEXT("/Game/PvPArena/VFX/Crystal/VFX/NS_Crystal_Torrent.NS_Crystal_Torrent");
    const FString ExpectedImpactEffectPath =
        TEXT("/Game/PvPArena/VFX/Crystal/VFX/NS_Crystal_Torrent_Owner_Cast.NS_Crystal_Torrent_Owner_Cast");
    const FBoolProperty* DebugEnabledProperty = FindFProperty<FBoolProperty>(APvPProjectile::StaticClass(), TEXT("bDrawProjectileDebug"));
    const FFloatProperty* DebugDurationProperty = FindFProperty<FFloatProperty>(APvPProjectile::StaticClass(), TEXT("ProjectileDebugDrawTime"));
    const FFloatProperty* DebugThicknessProperty = FindFProperty<FFloatProperty>(APvPProjectile::StaticClass(), TEXT("ProjectileDebugLineThickness"));
    const FClassProperty* ProjectileClassProperty = FindFProperty<FClassProperty>(UPvPCombatComponent::StaticClass(), TEXT("RangedProjectileClass"));
    const FObjectProperty* ImpactEffectProperty = FindFProperty<FObjectProperty>(APvPProjectile::StaticClass(), TEXT("ImpactEffect"));
    UFunction* ImpactMulticastFunction = APvPProjectile::StaticClass()->FindFunctionByName(TEXT("MulticastPlayImpactEffect"));
    TestNotNull(TEXT("Projectile should expose a draw-debug toggle"), DebugEnabledProperty);
    TestNotNull(TEXT("Projectile should expose a debug draw duration"), DebugDurationProperty);
    TestNotNull(TEXT("Projectile should expose a debug line thickness"), DebugThicknessProperty);
    TestNotNull(TEXT("Combat component should expose a ranged projectile class"), ProjectileClassProperty);
    TestNotNull(TEXT("Projectile should expose an impact effect default"), ImpactEffectProperty);
    TestNotNull(TEXT("Projectile should multicast impact effects so remote clients see hit feedback"), ImpactMulticastFunction);

    if (!DebugEnabledProperty || !DebugDurationProperty || !DebugThicknessProperty || !ProjectileClassProperty || !ImpactEffectProperty || !ImpactMulticastFunction)
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
    UStaticMeshComponent* ProjectileMesh = Projectile ? Projectile->FindComponentByClass<UStaticMeshComponent>() : nullptr;
    UNiagaraComponent* ProjectileEffect = Projectile ? Projectile->FindComponentByClass<UNiagaraComponent>() : nullptr;
    UNiagaraSystem* ImpactEffect = Projectile
        ? Cast<UNiagaraSystem>(ImpactEffectProperty->GetObjectPropertyValue_InContainer(Projectile))
        : nullptr;
    TestNotNull(TEXT("Projectile should have a projectile movement component"), ProjectileMovement);
    TestNotNull(TEXT("Projectile should create a visible orb mesh component"), ProjectileMesh);
    TestNotNull(TEXT("Projectile should create an energy shell Niagara component"), ProjectileEffect);
    TestNotNull(TEXT("Projectile should default an impact effect asset"), ImpactEffect);
    TestFalse(TEXT("Projectile travel Niagara should stay inactive until a loop-safe asset is assigned"), ProjectileEffect ? ProjectileEffect->bAutoActivate : true);
    TestEqual(
        TEXT("Projectile travel effect should use the assigned crystal torrent asset"),
        ProjectileEffect && ProjectileEffect->GetAsset() ? ProjectileEffect->GetAsset()->GetPathName() : FString(),
        ExpectedProjectileEffectPath);
    TestEqual(
        TEXT("Projectile impact should use the assigned crystal owner-cast asset"),
        ImpactEffect ? ImpactEffect->GetPathName() : FString(),
        ExpectedImpactEffectPath);

    TestTrue(TEXT("Projectile should ignore gravity for a straight magic bolt"), ProjectileMovement && ProjectileMovement->ProjectileGravityScale == 0.0f);
    TestEqual(TEXT("Projectile should slow down slightly for easier visual tracking"), ProjectileMovement ? ProjectileMovement->InitialSpeed : 0.0f, 1800.0f);
    TestEqual(TEXT("Projectile max speed should match the lowered initial speed"), ProjectileMovement ? ProjectileMovement->MaxSpeed : 0.0f, 1800.0f);
    TestTrue(TEXT("Projectile actor should replicate movement so remote clients follow the server path"), Projectile && Projectile->GetIsReplicated() && Projectile->IsReplicatingMovement());
    TestTrue(TEXT("Projectile impact effect RPC should replicate to all clients"), (ImpactMulticastFunction->FunctionFlags & FUNC_NetMulticast) != 0);
    TestTrue(TEXT("Projectile impact effect RPC should be reliable so hit feedback survives projectile destruction"), (ImpactMulticastFunction->FunctionFlags & FUNC_NetReliable) != 0);
    TestFalse(TEXT("Projectile draw-debug should default off so travel visuals stay readable"), DebugEnabledProperty->GetPropertyValue_InContainer(Projectile));
    TestEqual(TEXT("Projectile should keep its debug trail visible briefly"), DebugDurationProperty->GetPropertyValue_InContainer(Projectile), 1.0f);
    TestEqual(TEXT("Projectile should use a readable debug trail thickness"), DebugThicknessProperty->GetPropertyValue_InContainer(Projectile), 1.5f);

    return true;
}
