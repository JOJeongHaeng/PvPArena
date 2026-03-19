#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PvPCombatComponent.generated.h"

class APvPArenaCharacter;
class APvPProjectile;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PVPARENA_API UPvPCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPvPCombatComponent();

    float GetMeleeDamage() const { return MeleeDamage; }
    float GetRangedDamage() const { return RangedDamage; }
    float GetRangedCooldownSeconds() const { return RangedCooldownSeconds; }
    float GetRangedAimTraceDistance() const { return RangedRange; }
    float GetRemainingRangedCooldown(float NowSeconds) const;
    float GetRangedCooldownAlpha(float NowSeconds) const;

    bool CanUseMelee(float NowSeconds) const;
    bool CanUseRanged(float NowSeconds) const;

    void MarkMeleeUsed(float NowSeconds);
    void MarkRangedUsed(float NowSeconds);
    bool TryServerMeleeAttack(APvPArenaCharacter* Attacker);
    bool TryServerRangedAttack(APvPArenaCharacter* Attacker);
    static FVector BuildRangedProjectileSpawnLocation(const FVector& AimOrigin, const FVector& AimTarget, float ForwardOffset);

private:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float MeleeCooldownSeconds = 0.8f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float RangedCooldownSeconds = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float MeleeDamage = 25.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float RangedDamage = 20.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float MeleeRange = 140.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float MeleeRadius = 70.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float RangedRange = 2500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float RangedSpawnForwardOffset = 90.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float RangedSpawnHeightOffset = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    TSubclassOf<APvPProjectile> RangedProjectileClass;

    UPROPERTY(EditDefaultsOnly, Category = "Debug")
    bool bDrawAttackDebug = true;

    UPROPERTY(EditDefaultsOnly, Category = "Debug")
    float DebugDrawTime = 1.0f;

    float NextAllowedMeleeTime = 0.0f;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Combat")
    float NextAllowedRangedTime = 0.0f;
};
