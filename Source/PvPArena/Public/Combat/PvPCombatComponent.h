#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PvPCombatComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PVPARENA_API UPvPCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    bool CanUseMelee(float NowSeconds) const;
    bool CanUseRanged(float NowSeconds) const;

    void MarkMeleeUsed(float NowSeconds);
    void MarkRangedUsed(float NowSeconds);

private:
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float MeleeCooldownSeconds = 0.8f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float RangedCooldownSeconds = 1.2f;

    float NextAllowedMeleeTime = 0.0f;
    float NextAllowedRangedTime = 0.0f;
};
