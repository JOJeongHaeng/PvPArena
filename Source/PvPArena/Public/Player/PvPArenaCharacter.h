#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PvPArenaCharacter.generated.h"

class AController;
class UAnimationAsset;
class UAnimMontage;
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
    bool IsInvulnerable() const { return bIsInvulnerable; }
    float GetDeathAnimationDuration() const;
    bool IsMeleeAttackInProgress() const { return bMeleeAttackInProgress; }
    bool HasTriggeredMeleeAttackHit() const { return bMeleeAttackHitTriggered; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyServerDamage(float Damage, AController* InstigatorController);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetInvulnerableForSeconds(float DurationSeconds);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool BeginMeleeAttack(float NowSeconds);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool HandleMeleeAttackHitNotify();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void FinishMeleeAttack();

    void ShowMeleeDebug(FVector Start, FVector End, bool bHit);

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

    UFUNCTION()
    void OnRep_IsInvulnerable();

private:
    bool TriggerMeleeAttackHit();
    void PlayDeathAnimation();
    bool PlayMeleeAttackMontage();
    void HandleMeleeAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    void SetMeleeMovementSuppressed(bool bSuppressed);
    void SetDeathInputSuppressed(bool bSuppressInput);
    void TryApplyInputMappingContext();

    UFUNCTION(Server, Reliable)
    void ServerHandleMeleeAttackHitNotify();

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastPlayMeleeAttackMontage();

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastDrawMeleeDebug(FVector Start, FVector End, bool bHit);

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float MaxHealth = 100.0f;

    UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_CurrentHealth, Category = "Combat")
    float CurrentHealth = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_IsDead, VisibleAnywhere, Category = "Combat")
    bool bIsDead = false;

    UPROPERTY(ReplicatedUsing = OnRep_IsInvulnerable, VisibleAnywhere, Category = "Combat")
    bool bIsInvulnerable = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UPvPCombatComponent> CombatComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultInputMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    TObjectPtr<UAnimationAsset> DeathAnimation;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    TObjectPtr<UAnimMontage> MeleeAttackMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Animation", meta = (ClampMin = "0.1"))
    float MeleeAttackPlayRate = 2.0f;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    bool bMeleeAttackInProgress = false;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    bool bMeleeAttackHitTriggered = false;

    uint8 CachedMovementMode = MOVE_Walking;
    bool bMeleeMovementSuppressed = false;

    FTimerHandle InvulnerabilityTimerHandle;
};
