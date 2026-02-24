#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PvPCombatComponent.generated.h"

class APvPArenaCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PVPARENA_API UPvPCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    float GetMeleeDamage() const { return MeleeDamage; }
    float GetRangedDamage() const { return RangedDamage; }

    bool CanUseMelee(float NowSeconds) const;
    bool CanUseRanged(float NowSeconds) const;

    void MarkMeleeUsed(float NowSeconds);
    void MarkRangedUsed(float NowSeconds);
    bool TryServerMeleeAttack(APvPArenaCharacter* Attacker);
    bool TryServerRangedAttack(APvPArenaCharacter* Attacker);

private:
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float MeleeCooldownSeconds = 0.8f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float RangedCooldownSeconds = 1.2f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float MeleeDamage = 25.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float RangedDamage = 20.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float MeleeRange = 180.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float MeleeRadius = 70.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float RangedRange = 2500.0f;

    float NextAllowedMeleeTime = 0.0f;
    float NextAllowedRangedTime = 0.0f;
};
