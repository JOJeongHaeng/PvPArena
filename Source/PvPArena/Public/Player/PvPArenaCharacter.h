#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PvPArenaCharacter.generated.h"

UCLASS()
class PVPARENA_API APvPArenaCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APvPArenaCharacter();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyDamageServer(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Respawn")
    bool CanRespawnAtTime(float ServerTimeSeconds) const;

    UFUNCTION(BlueprintCallable, Category = "Respawn")
    void RespawnToFullHealth();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
    float MaxHealth;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Health")
    float CurrentHealth;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Health")
    bool bIsDead;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn")
    float RespawnDelaySeconds;

private:
    void EnterDeadState();

    float DeathServerTimeSeconds;
};
