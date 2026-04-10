#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UObject/SoftObjectPtr.h"
#include "PvPArenaCharacter.generated.h"

class AController;
class UAudioComponent;
class UAnimationAsset;
class UAnimMontage;
class UInputMappingContext;
class UNiagaraSystem;
class UPvPCombatComponent;
class USpringArmComponent;
class USoundBase;
class UWidgetComponent;

enum class ERangedHitNotifyHandling : uint8
{
    Ignore,
    SendToServer,
    TriggerImmediately
};

UCLASS()
class PVPARENA_API APvPArenaCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APvPArenaCharacter();

    float GetCurrentHealth() const { return CurrentHealth; }
    float GetMaxHealth() const { return MaxHealth; }
    float GetSprintDurationSeconds() const { return SprintDurationSeconds; }
    float GetSprintSpeedMultiplier() const { return SprintSpeedMultiplier; }
    float GetSprintRechargeRate() const { return SprintRechargeRate; }
    float GetCurrentSprintEnergySeconds() const { return CurrentSprintEnergySeconds; }
    float GetSprintEnergyAlpha() const;
    bool IsDead() const { return bIsDead; }
    bool IsInvulnerable() const { return bIsInvulnerable; }
    bool IsSprintInputHeld() const { return bSprintInputHeld; }
    bool IsSprinting() const { return bSprintActive; }
    float GetDeathAnimationDuration() const;
    UPvPCombatComponent* GetCombatComponent() const { return CombatComponent; }
    bool IsMeleeAttackInProgress() const { return bMeleeAttackInProgress; }
    bool HasTriggeredMeleeAttackHit() const { return bMeleeAttackHitTriggered; }
    bool IsRangedAttackInProgress() const { return bRangedAttackInProgress; }
    bool HasTriggeredRangedAttackHit() const { return bRangedAttackHitTriggered; }
    float GetRangedAttackTargetYaw() const { return RangedAttackTargetYaw; }
    FName GetMeleeAttackSocketName() const { return MeleeAttackSocketName; }
    bool IsRangedReleaseCommitted() const { return bRangedReleaseCommitted; }
    bool IsRangedChargeInputHeld() const { return bRangedChargeInputHeld; }
    float GetRangedAimCameraBlendAlpha() const { return RangedAimCameraBlendAlpha; }
    UWidgetComponent* GetOverheadStatusWidgetComponent() const { return OverheadStatusWidgetComponent; }
    bool ResolveRangedCrosshairAimPoint(FVector& OutAimPoint) const;
    bool GetCachedRangedAttackAim(FVector& OutAimOrigin, FVector& OutAimTarget) const;
    void ApplyAudioSettings(float InMasterVolume, float InSfxVolume);
    static bool BuildOverheadWidgetVisibleState(
        bool bHasLocalViewController,
        bool bIsLocallyControlledCharacter,
        bool bIsTargetInFrontOfCamera,
        bool bHasBlockingHit,
        bool bHitOwningCharacter);
    static ERangedHitNotifyHandling ResolveRangedHitNotifyHandling(
        bool bHasAuthority,
        bool bIsLocallyControlled,
        bool bControllerIsPlayer);

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

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool BeginRangedAttack(float NowSeconds);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool BeginRangedCharge(float NowSeconds);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ReleaseRangedCharge(float NowSeconds);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool HandleRangedAttackHitNotify();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void FinishRangedAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AdvanceAttackFacing(float DeltaSeconds);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void BeginSprintInput();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void EndSprintInput();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateSprintState(float DeltaSeconds);

    void ShowMeleeDebug(FVector Start, FVector End, bool bHit);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Combat")
    void ServerTryMeleeAttack();

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Combat")
    void ServerTryRangedAttack();

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Combat")
    void ServerBeginRangedCharge();

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Combat")
    void ServerReleaseRangedCharge();

protected:
    virtual void PostInitializeComponents() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void BeginPlay() override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_Controller() override;
    virtual void OnRep_PlayerState() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UFUNCTION()
    void OnRep_CurrentHealth();

    UFUNCTION()
    void OnRep_IsDead();

    UFUNCTION()
    void OnRep_IsInvulnerable();

private:
    bool TriggerMeleeAttackHit();
    bool TriggerRangedAttackHit();
    void CommitRangedChargeRelease();
    void CancelRangedCharge();
    bool JumpToRangedMontageSection(FName SectionName);
    void UpdateRangedAimCameraOffset(float DeltaSeconds);
    USpringArmComponent* ResolveRangedAimSpringArm();
    void RefreshRangedAttackAimFromCrosshair();
    bool ResolveRangedCrosshairAim(FVector& OutAimOrigin, FVector& OutAimPoint) const;
    void CacheRangedAttackAim(const FVector& AimOrigin, const FVector& AimTarget);
    void ClearCachedRangedAttackAim();
    bool CaptureCurrentRangedAttackAim();
    float ResolveRangedAttackTargetYaw() const;
    void StartRangedAttackFacingLock(float TargetYaw);
    void UpdateRangedAttackFacing(float DeltaSeconds);
    void PlayDeathAnimation();
    void PlayMeleeAttackSound();
    void StopMeleeAttackSound();
    void PlayRangedAttackSound();
    void RefreshAttackAudioVolumes();
    void PlayMeleeAttackEffect();
    bool PlayMeleeAttackMontage();
    bool PlayRangedAttackMontage(FName StartSectionName = NAME_None);
    void HandleMeleeAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    void HandleRangedAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    void SetAttackMovementSuppressed(bool bSuppressed);
    void SetDeathInputSuppressed(bool bSuppressInput);
    void RefreshSprintMovementSpeed();
    bool CanSprint() const;
    bool TryActivateSprintDash();
    FVector ResolveSprintDashDirection() const;
    void ApplySprintDashVelocity(const FVector& DashDirection);
    float ResolveSprintDashTargetYaw() const;
    bool UpdatePendingSprintDash(float DeltaSeconds);
    void TryApplyInputMappingContext();
    void RefreshOverheadStatusWidget();
    TSubclassOf<class UUserWidget> ResolveOverheadStatusWidgetClass();
    FName ResolveOverheadWidgetSocketName() const;
    void RefreshOverheadWidgetAttachment();
    void RefreshOverheadWidgetFacing();
    void RefreshOverheadWidgetVisibility();
    bool ShouldShowOverheadWidgetForLocalView(const APlayerController* LocalPlayerController) const;
    FVector ResolveOverheadWidgetViewTarget() const;

    UFUNCTION(Server, Reliable)
    void ServerHandleMeleeAttackHitNotify();

    UFUNCTION(Server, Reliable)
    void ServerHandleRangedAttackHitNotify(bool bHasAim, FVector_NetQuantize AimOrigin, FVector_NetQuantize AimTarget);

    UFUNCTION(Server, Reliable)
    void ServerSetSprintInputHeld(bool bNewSprintInputHeld);

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastPlayMeleeAttackMontage();

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastPlayRangedAttackMontage(float TargetYaw);

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastResolveRangedCharge(bool bCommitted);

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastDrawMeleeDebug(FVector Start, FVector End, bool bHit);

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastPlayRangedAttackSound();

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

    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    TObjectPtr<UNiagaraSystem> MeleeAttackEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<USoundBase> MeleeAttackSound;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<USoundBase> RangedAttackSound;

    UPROPERTY(EditDefaultsOnly, Category = "Audio", meta = (ClampMin = "0.1"))
    float MeleeAttackSoundDurationSeconds = 1.25f;

    UPROPERTY(EditDefaultsOnly, Category = "Audio", meta = (ClampMin = "0.0"))
    float RangedAttackSoundVolume = 0.7f;

    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    FName MeleeAttackEffectSocketName = TEXT("hand_l");

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    FName MeleeAttackSocketName = TEXT("hand_l");

    UPROPERTY(EditDefaultsOnly, Category = "Animation", meta = (ClampMin = "0.1"))
    float MeleeAttackPlayRate = 2.75f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    TObjectPtr<UAnimMontage> RangedAttackMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Animation", meta = (ClampMin = "0.1"))
    float RangedAttackPlayRate = 1.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "0.1"))
    float SprintSpeedMultiplier = 1.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "0.1"))
    float SprintDurationSeconds = 1.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "0.1"))
    float SprintRechargeRate = 0.75f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "0.1"))
    float SprintDashDistance = 1800.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "0.0"))
    float SprintActiveDurationSeconds = 0.15f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "0.0"))
    float SprintDashTurnInterpSpeed = 720.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "0.0"))
    float SprintDashYawToleranceDegrees = 6.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    FName RangedAttackStartSectionName = TEXT("Start");

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    FName RangedAttackHoldSectionName = TEXT("Hold");

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    FName RangedAttackReleaseSectionName = TEXT("Release");

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    FName RangedAttackCancelSectionName = TEXT("Cancel");

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    bool bMeleeAttackInProgress = false;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    bool bMeleeAttackHitTriggered = false;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    bool bRangedAttackInProgress = false;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    bool bRangedChargeInputHeld = false;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    bool bRangedReleaseCommitted = false;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    bool bRangedAttackHitTriggered = false;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    float RangedChargeStartTime = 0.0f;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    bool bRangedAttackFacingLocked = false;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    float RangedAttackTargetYaw = 0.0f;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    bool bHasCachedRangedAttackAim = false;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    bool bSprintInputHeld = false;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    bool bSprintActive = false;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    bool bSprintDepletedLocked = false;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float CurrentSprintEnergySeconds = 3.0f;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float BaseWalkSpeed = 0.0f;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float SprintActiveTimeRemaining = 0.0f;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    bool bPendingSprintDash = false;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float PendingSprintDashTargetYaw = 0.0f;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    FVector CachedRangedAttackAimOrigin = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    FVector CachedRangedAttackAimTarget = FVector::ZeroVector;

    UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (ClampMin = "1.0"))
    float RangedAttackTurnInterpSpeed = 12.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (ClampMin = "0.0"))
    float RangedChargeMinimumHoldSeconds = 0.1f;

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float RangedAimCameraOffsetY = 60.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (ClampMin = "0.0"))
    float RangedAimCameraInterpSpeed = 6.0f;

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    float RangedAimCameraBlendAlpha = 0.0f;

    UPROPERTY(VisibleAnywhere, Category = "Audio")
    TObjectPtr<UAudioComponent> MeleeAttackAudioComponent;

    UPROPERTY(VisibleAnywhere, Category = "Audio")
    TObjectPtr<UAudioComponent> RangedAttackAudioComponent;

    UPROPERTY(VisibleAnywhere, Category = "UI")
    TObjectPtr<UWidgetComponent> OverheadStatusWidgetComponent;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSoftClassPtr<UUserWidget> OverheadStatusWidgetClass;

    UPROPERTY(Transient)
    TObjectPtr<USpringArmComponent> RangedAimSpringArm;

    FVector RangedAimCameraBaseSocketOffset = FVector::ZeroVector;
    bool bRangedAimCameraBaseSocketOffsetCached = false;

    uint8 CachedMovementMode = MOVE_Walking;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    bool bAttackMovementSuppressed = false;

    FTimerHandle MeleeAttackAudioTimerHandle;
    FTimerHandle InvulnerabilityTimerHandle;
    float CurrentMasterVolume = 1.0f;
    float CurrentSfxVolume = 1.0f;
};
