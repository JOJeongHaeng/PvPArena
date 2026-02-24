#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PvPArenaCharacter.generated.h"

class AController;
class UInputMappingContext;
class UPvPCombatComponent;

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

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Combat")
    void ServerTryMeleeAttack();

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Combat")
    void ServerTryRangedAttack();

protected:
    virtual void BeginPlay() override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_Controller() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_CurrentHealth();

    UFUNCTION()
    void OnRep_IsDead();

private:
    void TryApplyInputMappingContext();

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float MaxHealth = 100.0f;

    UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_CurrentHealth, Category = "Combat")
    float CurrentHealth = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_IsDead, VisibleAnywhere, Category = "Combat")
    bool bIsDead = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UPvPCombatComponent> CombatComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultInputMappingContext;
};
