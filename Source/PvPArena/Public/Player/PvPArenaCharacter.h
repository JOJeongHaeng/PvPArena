#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PvPArenaCharacter.generated.h"

class AController;

UCLASS()
class PVPARENA_API APvPArenaCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APvPArenaCharacter();

    float GetCurrentHealth() const { return CurrentHealth; }
    float GetMaxHealth() const { return MaxHealth; }
    bool IsDead() const { return bIsDead; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyServerDamage(float Damage, AController* InstigatorController);

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_CurrentHealth();

    UFUNCTION()
    void OnRep_IsDead();

private:
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float MaxHealth = 100.0f;

    UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_CurrentHealth, Category = "Combat")
    float CurrentHealth = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_IsDead, VisibleAnywhere, Category = "Combat")
    bool bIsDead = false;
};
