#include "Player/PvPArenaCharacter.h"

#include "Animation/AnimationAsset.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/AudioComponent.h"
#include "Combat/PvPCombatComponent.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Game/PvPArenaGameMode.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputCoreTypes.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Net/UnrealNetwork.h"
#include "PvPArena.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

APvPArenaCharacter::APvPArenaCharacter()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = true;
    CombatComponent = CreateDefaultSubobject<UPvPCombatComponent>(TEXT("CombatComponent"));
    MeleeAttackAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MeleeAttackAudioComponent"));
    RangedAttackAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("RangedAttackAudioComponent"));

    if (MeleeAttackAudioComponent)
    {
        MeleeAttackAudioComponent->SetupAttachment(GetRootComponent());
        MeleeAttackAudioComponent->SetAutoActivate(false);
    }

    if (RangedAttackAudioComponent)
    {
        RangedAttackAudioComponent->SetupAttachment(GetRootComponent());
        RangedAttackAudioComponent->SetAutoActivate(false);
    }

    static ConstructorHelpers::FObjectFinder<UAnimationAsset> DeathAnimationFinder(
        TEXT("/Game/PvPArena/Animations/Stand_Relaxed_Death1.Stand_Relaxed_Death1"));
    if (DeathAnimationFinder.Succeeded())
    {
        DeathAnimation = DeathAnimationFinder.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimMontage> MeleeAttackMontageFinder(
        TEXT("/Game/PvPArena/Animations/MTG_MeleeAttack01.MTG_MeleeAttack01"));
    if (MeleeAttackMontageFinder.Succeeded())
    {
        MeleeAttackMontage = MeleeAttackMontageFinder.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimMontage> RangedAttackMontageFinder(
        TEXT("/Game/PvPArena/Animations/MTG_RangedAttack_RightClick.MTG_RangedAttack_RightClick"));
    if (RangedAttackMontageFinder.Succeeded())
    {
        RangedAttackMontage = RangedAttackMontageFinder.Object;
    }

    static ConstructorHelpers::FObjectFinder<UNiagaraSystem> MeleeAttackEffectFinder(
        TEXT("/Game/PvPArena/VFX/fire/VFX/NS_Magma_Shot_Owner_Cast_Spell.NS_Magma_Shot_Owner_Cast_Spell"));
    if (MeleeAttackEffectFinder.Succeeded())
    {
        MeleeAttackEffect = MeleeAttackEffectFinder.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> MeleeAttackSoundFinder(
        TEXT("/Game/PvPArena/Audio/Fire01_Cue.Fire01_Cue"));
    if (MeleeAttackSoundFinder.Succeeded())
    {
        MeleeAttackSound = MeleeAttackSoundFinder.Object;
        if (MeleeAttackAudioComponent)
        {
            MeleeAttackAudioComponent->SetSound(MeleeAttackSound);
        }
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> RangedAttackSoundFinder(
        TEXT("/Game/PvPArena/Audio/Explosion_Cue.Explosion_Cue"));
    if (RangedAttackSoundFinder.Succeeded())
    {
        RangedAttackSound = RangedAttackSoundFinder.Object;
        if (RangedAttackAudioComponent)
        {
            RangedAttackAudioComponent->SetSound(RangedAttackSound);
        }
    }

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        BaseWalkSpeed = MoveComp->MaxWalkSpeed;
        CurrentSprintEnergySeconds = SprintDurationSeconds;
    }
}

void APvPArenaCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (HasAuthority() || IsLocallyControlled())
    {
        UpdateSprintState(DeltaSeconds);
    }
    RefreshRangedAttackAimFromCrosshair();
    UpdateRangedAttackFacing(DeltaSeconds);
    UpdateRangedAimCameraOffset(DeltaSeconds);
}

void APvPArenaCharacter::BeginPlay()
{
    Super::BeginPlay();
    CurrentSprintEnergySeconds = FMath::Clamp(CurrentSprintEnergySeconds, 0.0f, SprintDurationSeconds);
    RefreshSprintMovementSpeed();
    SetDeathInputSuppressed(bIsDead);
    TryApplyInputMappingContext();
}

void APvPArenaCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    SetDeathInputSuppressed(bIsDead);
    TryApplyInputMappingContext();
}

void APvPArenaCharacter::OnRep_Controller()
{
    Super::OnRep_Controller();
    SetDeathInputSuppressed(bIsDead);
    TryApplyInputMappingContext();
}

void APvPArenaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (!PlayerInputComponent)
    {
        return;
    }

    PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &APvPArenaCharacter::BeginSprintInput);
    PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Released, this, &APvPArenaCharacter::EndSprintInput);
    PlayerInputComponent->BindKey(EKeys::RightShift, IE_Pressed, this, &APvPArenaCharacter::BeginSprintInput);
    PlayerInputComponent->BindKey(EKeys::RightShift, IE_Released, this, &APvPArenaCharacter::EndSprintInput);
}

void APvPArenaCharacter::OnRep_CurrentHealth()
{
    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
}

void APvPArenaCharacter::OnRep_IsDead()
{
    SetDeathInputSuppressed(bIsDead);

    if (!bIsDead)
    {
        return;
    }

    PlayDeathAnimation();
}

void APvPArenaCharacter::OnRep_IsInvulnerable()
{
}

float APvPArenaCharacter::GetSprintEnergyAlpha() const
{
    return SprintDurationSeconds > 0.0f
        ? FMath::Clamp(CurrentSprintEnergySeconds / SprintDurationSeconds, 0.0f, 1.0f)
        : 0.0f;
}

void APvPArenaCharacter::TryApplyInputMappingContext()
{
    if (!DefaultInputMappingContext || !IsLocallyControlled())
    {
        return;
    }

    APlayerController* PlayerController = Cast<APlayerController>(Controller);
    if (!PlayerController)
    {
        return;
    }

    ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
    if (!LocalPlayer)
    {
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
    if (!InputSubsystem)
    {
        return;
    }

    InputSubsystem->AddMappingContext(DefaultInputMappingContext, 0);
}

void APvPArenaCharacter::BeginSprintInput()
{
    bSprintInputHeld = true;
    if (bSprintDepletedLocked)
    {
        return;
    }

    if (!HasAuthority())
    {
        ServerSetSprintInputHeld(true);
    }
}

void APvPArenaCharacter::EndSprintInput()
{
    bSprintInputHeld = false;
    bSprintActive = false;
    RefreshSprintMovementSpeed();

    if (!HasAuthority())
    {
        ServerSetSprintInputHeld(false);
    }
}

void APvPArenaCharacter::UpdateSprintState(float DeltaSeconds)
{
    if (DeltaSeconds <= 0.0f)
    {
        RefreshSprintMovementSpeed();
        return;
    }

    const bool bCanSprintNow = CanSprint();
    if (bSprintInputHeld && !bSprintDepletedLocked && bCanSprintNow && CurrentSprintEnergySeconds > 0.0f)
    {
        bSprintActive = true;
        CurrentSprintEnergySeconds = FMath::Max(0.0f, CurrentSprintEnergySeconds - DeltaSeconds);
        if (CurrentSprintEnergySeconds <= 0.0f)
        {
            bSprintActive = false;
            bSprintDepletedLocked = true;
        }
    }
    else
    {
        bSprintActive = false;
        CurrentSprintEnergySeconds = FMath::Min(SprintDurationSeconds, CurrentSprintEnergySeconds + (DeltaSeconds * SprintRechargeRate));
        if (CurrentSprintEnergySeconds >= SprintDurationSeconds)
        {
            bSprintDepletedLocked = false;
        }
    }

    RefreshSprintMovementSpeed();
}

void APvPArenaCharacter::PlayDeathAnimation()
{
    if (!DeathAnimation || !GetMesh())
    {
        return;
    }

    GetMesh()->PlayAnimation(DeathAnimation, false);
}

void APvPArenaCharacter::SetDeathInputSuppressed(bool bSuppressInput)
{
    APlayerController* PlayerController = Cast<APlayerController>(Controller);
    if (!PlayerController)
    {
        return;
    }

    if (bSuppressInput)
    {
        DisableInput(PlayerController);
    }
    else
    {
        EnableInput(PlayerController);
    }

    PlayerController->SetIgnoreMoveInput(bSuppressInput);
    PlayerController->SetIgnoreLookInput(bSuppressInput);
    if (bSuppressInput)
    {
        bSprintActive = false;
        bSprintInputHeld = false;
        RefreshSprintMovementSpeed();
    }
}

float APvPArenaCharacter::GetDeathAnimationDuration() const
{
    return DeathAnimation ? DeathAnimation->GetPlayLength() : 0.0f;
}

bool APvPArenaCharacter::BeginMeleeAttack(float NowSeconds)
{
    if (!CombatComponent || bIsDead || bMeleeAttackInProgress || bRangedAttackInProgress || !CombatComponent->CanUseMelee(NowSeconds))
    {
        return false;
    }

    CombatComponent->MarkMeleeUsed(NowSeconds);
    bMeleeAttackInProgress = true;
    bMeleeAttackHitTriggered = false;
    SetAttackMovementSuppressed(true);
    return true;
}

bool APvPArenaCharacter::HandleMeleeAttackHitNotify()
{
    if (!IsLocallyControlled() && !HasAuthority())
    {
        return true;
    }

    if (!HasAuthority())
    {
        ServerHandleMeleeAttackHitNotify();
        return true;
    }

    return TriggerMeleeAttackHit();
}

bool APvPArenaCharacter::TriggerMeleeAttackHit()
{
    if (!bMeleeAttackInProgress || bMeleeAttackHitTriggered)
    {
        return false;
    }

    bMeleeAttackHitTriggered = true;
    return CombatComponent ? CombatComponent->TryServerMeleeAttack(this) : false;
}

void APvPArenaCharacter::FinishMeleeAttack()
{
    bMeleeAttackInProgress = false;
    bMeleeAttackHitTriggered = false;
    if (!bRangedAttackInProgress)
    {
        SetAttackMovementSuppressed(false);
    }
}

bool APvPArenaCharacter::BeginRangedAttack(float NowSeconds)
{
    if (!BeginRangedCharge(NowSeconds))
    {
        return false;
    }

    CommitRangedChargeRelease();
    if (CombatComponent)
    {
        CombatComponent->MarkRangedUsed(NowSeconds);
    }
    return true;
}

bool APvPArenaCharacter::BeginRangedCharge(float NowSeconds)
{
    if (!CombatComponent || bIsDead || bMeleeAttackInProgress || bRangedAttackInProgress || !CombatComponent->CanUseRanged(NowSeconds))
    {
        UE_LOG(
            LogPvPArena,
            Warning,
            TEXT("BeginRangedCharge blocked Character=%s Role=%d Authority=%d Local=%d Combat=%d Dead=%d MeleeInProgress=%d RangedInProgress=%d CanUse=%d"),
            *GetName(),
            static_cast<int32>(GetLocalRole()),
            HasAuthority(),
            IsLocallyControlled(),
            CombatComponent != nullptr,
            bIsDead,
            bMeleeAttackInProgress,
            bRangedAttackInProgress,
            CombatComponent ? CombatComponent->CanUseRanged(NowSeconds) : 0);
        return false;
    }

    bRangedAttackInProgress = true;
    bRangedChargeInputHeld = true;
    bRangedReleaseCommitted = false;
    bRangedAttackHitTriggered = false;
    RangedChargeStartTime = NowSeconds;
    ClearCachedRangedAttackAim();
    StartRangedAttackFacingLock(ResolveRangedAttackTargetYaw());
    SetAttackMovementSuppressed(true);
    UE_LOG(
        LogPvPArena,
        Log,
        TEXT("BeginRangedCharge Character=%s Role=%d Authority=%d Local=%d TargetYaw=%.2f"),
        *GetName(),
        static_cast<int32>(GetLocalRole()),
        HasAuthority(),
        IsLocallyControlled(),
        RangedAttackTargetYaw);
    return true;
}

bool APvPArenaCharacter::ReleaseRangedCharge(float NowSeconds)
{
    if (!bRangedAttackInProgress)
    {
        UE_LOG(
            LogPvPArena,
            Warning,
            TEXT("ReleaseRangedCharge blocked Character=%s Role=%d Authority=%d Local=%d Reason=NotInProgress"),
            *GetName(),
            static_cast<int32>(GetLocalRole()),
            HasAuthority(),
            IsLocallyControlled());
        return false;
    }

    bRangedChargeInputHeld = false;
    if ((NowSeconds - RangedChargeStartTime) < RangedChargeMinimumHoldSeconds)
    {
        UE_LOG(
            LogPvPArena,
            Warning,
            TEXT("ReleaseRangedCharge cancel Character=%s Role=%d Authority=%d Local=%d HeldFor=%.3f Minimum=%.3f"),
            *GetName(),
            static_cast<int32>(GetLocalRole()),
            HasAuthority(),
            IsLocallyControlled(),
            NowSeconds - RangedChargeStartTime,
            RangedChargeMinimumHoldSeconds);
        CancelRangedCharge();
        return false;
    }

    const bool bCapturedAim = CaptureCurrentRangedAttackAim();
    CommitRangedChargeRelease();
    if (CombatComponent)
    {
        CombatComponent->MarkRangedUsed(NowSeconds);
    }

    UE_LOG(
        LogPvPArena,
        Log,
        TEXT("ReleaseRangedCharge Character=%s Role=%d Authority=%d Local=%d CapturedAim=%d CachedAim=%d AimOrigin=%s AimTarget=%s"),
        *GetName(),
        static_cast<int32>(GetLocalRole()),
        HasAuthority(),
        IsLocallyControlled(),
        bCapturedAim,
        bHasCachedRangedAttackAim,
        *CachedRangedAttackAimOrigin.ToCompactString(),
        *CachedRangedAttackAimTarget.ToCompactString());

    return true;
}

bool APvPArenaCharacter::HandleRangedAttackHitNotify()
{
    const ERangedHitNotifyHandling NotifyHandling = ResolveRangedHitNotifyHandling(
        HasAuthority(),
        IsLocallyControlled(),
        Controller && Controller->IsPlayerController());

    UE_LOG(
        LogPvPArena,
        Log,
        TEXT("HandleRangedAttackHitNotify Character=%s Role=%d RemoteRole=%d Authority=%d Local=%d ControllerPlayer=%d Handling=%d InProgress=%d ReleaseCommitted=%d HitTriggered=%d CachedAim=%d AimOrigin=%s AimTarget=%s"),
        *GetName(),
        static_cast<int32>(GetLocalRole()),
        static_cast<int32>(GetRemoteRole()),
        HasAuthority(),
        IsLocallyControlled(),
        Controller && Controller->IsPlayerController(),
        static_cast<int32>(NotifyHandling),
        bRangedAttackInProgress,
        bRangedReleaseCommitted,
        bRangedAttackHitTriggered,
        bHasCachedRangedAttackAim,
        *CachedRangedAttackAimOrigin.ToCompactString(),
        *CachedRangedAttackAimTarget.ToCompactString());

    if (NotifyHandling == ERangedHitNotifyHandling::Ignore)
    {
        return true;
    }

    if (NotifyHandling == ERangedHitNotifyHandling::SendToServer)
    {
        const bool bHasAimForServer = bHasCachedRangedAttackAim || CaptureCurrentRangedAttackAim();
        UE_LOG(
            LogPvPArena,
            Log,
            TEXT("HandleRangedAttackHitNotify send-to-server Character=%s Role=%d HasAimForServer=%d CachedAim=%d AimOrigin=%s AimTarget=%s"),
            *GetName(),
            static_cast<int32>(GetLocalRole()),
            bHasAimForServer,
            bHasCachedRangedAttackAim,
            *CachedRangedAttackAimOrigin.ToCompactString(),
            *CachedRangedAttackAimTarget.ToCompactString());
        ServerHandleRangedAttackHitNotify(bHasAimForServer, CachedRangedAttackAimOrigin, CachedRangedAttackAimTarget);
        return true;
    }

    return TriggerRangedAttackHit();
}

bool APvPArenaCharacter::TriggerRangedAttackHit()
{
    if (!bRangedAttackInProgress || bRangedAttackHitTriggered || !bRangedReleaseCommitted)
    {
        UE_LOG(
            LogPvPArena,
            Warning,
            TEXT("TriggerRangedAttackHit blocked Character=%s Role=%d Authority=%d InProgress=%d ReleaseCommitted=%d HitTriggered=%d CachedAim=%d"),
            *GetName(),
            static_cast<int32>(GetLocalRole()),
            HasAuthority(),
            bRangedAttackInProgress,
            bRangedReleaseCommitted,
            bRangedAttackHitTriggered,
            bHasCachedRangedAttackAim);
        return false;
    }

    bRangedAttackHitTriggered = true;
    MulticastPlayRangedAttackSound();
    const bool bLaunched = CombatComponent ? CombatComponent->TryServerRangedAttack(this) : false;
    UE_LOG(
        LogPvPArena,
        Log,
        TEXT("TriggerRangedAttackHit Character=%s Role=%d Authority=%d LaunchResult=%d CachedAim=%d AimOrigin=%s AimTarget=%s"),
        *GetName(),
        static_cast<int32>(GetLocalRole()),
        HasAuthority(),
        bLaunched,
        bHasCachedRangedAttackAim,
        *CachedRangedAttackAimOrigin.ToCompactString(),
        *CachedRangedAttackAimTarget.ToCompactString());
    return bLaunched;
}

void APvPArenaCharacter::CommitRangedChargeRelease()
{
    bRangedReleaseCommitted = true;
    JumpToRangedMontageSection(RangedAttackReleaseSectionName);
}

void APvPArenaCharacter::CancelRangedCharge()
{
    bRangedReleaseCommitted = false;
    JumpToRangedMontageSection(RangedAttackCancelSectionName);
}

bool APvPArenaCharacter::JumpToRangedMontageSection(FName SectionName)
{
    if (SectionName.IsNone() || !RangedAttackMontage || !GetMesh())
    {
        return false;
    }

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance || !AnimInstance->Montage_IsPlaying(RangedAttackMontage))
    {
        return false;
    }

    AnimInstance->Montage_JumpToSection(SectionName, RangedAttackMontage);
    return true;
}

void APvPArenaCharacter::UpdateRangedAimCameraOffset(float DeltaSeconds)
{
    const float TargetBlendAlpha = bRangedChargeInputHeld ? 1.0f : 0.0f;
    RangedAimCameraBlendAlpha = FMath::FInterpTo(
        RangedAimCameraBlendAlpha,
        TargetBlendAlpha,
        DeltaSeconds,
        RangedAimCameraInterpSpeed);

    USpringArmComponent* SpringArm = ResolveRangedAimSpringArm();
    if (!SpringArm)
    {
        return;
    }

    if (!bRangedAimCameraBaseSocketOffsetCached)
    {
        RangedAimCameraBaseSocketOffset = SpringArm->SocketOffset;
        bRangedAimCameraBaseSocketOffsetCached = true;
    }

    FVector TargetSocketOffset = RangedAimCameraBaseSocketOffset;
    TargetSocketOffset.Y += RangedAimCameraOffsetY * RangedAimCameraBlendAlpha;
    SpringArm->SocketOffset = TargetSocketOffset;
}

USpringArmComponent* APvPArenaCharacter::ResolveRangedAimSpringArm()
{
    if (RangedAimSpringArm)
    {
        return RangedAimSpringArm;
    }

    RangedAimSpringArm = FindComponentByClass<USpringArmComponent>();
    return RangedAimSpringArm;
}

void APvPArenaCharacter::FinishRangedAttack()
{
    bRangedAttackInProgress = false;
    bRangedChargeInputHeld = false;
    bRangedReleaseCommitted = false;
    bRangedAttackHitTriggered = false;
    RangedChargeStartTime = 0.0f;
    bRangedAttackFacingLocked = false;
    ClearCachedRangedAttackAim();
    if (!bMeleeAttackInProgress)
    {
        SetAttackMovementSuppressed(false);
    }
}

void APvPArenaCharacter::AdvanceAttackFacing(float DeltaSeconds)
{
    RefreshRangedAttackAimFromCrosshair();
    UpdateRangedAttackFacing(DeltaSeconds);
    UpdateRangedAimCameraOffset(DeltaSeconds);
}

bool APvPArenaCharacter::ResolveRangedCrosshairAimPoint(FVector& OutAimPoint) const
{
    FVector AimOrigin = FVector::ZeroVector;
    return ResolveRangedCrosshairAim(AimOrigin, OutAimPoint);
}

bool APvPArenaCharacter::GetCachedRangedAttackAim(FVector& OutAimOrigin, FVector& OutAimTarget) const
{
    if (!bHasCachedRangedAttackAim)
    {
        return false;
    }

    OutAimOrigin = CachedRangedAttackAimOrigin;
    OutAimTarget = CachedRangedAttackAimTarget;
    return true;
}

ERangedHitNotifyHandling APvPArenaCharacter::ResolveRangedHitNotifyHandling(
    bool bHasAuthority,
    bool bIsLocallyControlled,
    bool bControllerIsPlayer)
{
    if (!bHasAuthority)
    {
        return bIsLocallyControlled
            ? ERangedHitNotifyHandling::SendToServer
            : ERangedHitNotifyHandling::Ignore;
    }

    if (bIsLocallyControlled || !bControllerIsPlayer)
    {
        return ERangedHitNotifyHandling::TriggerImmediately;
    }

    return ERangedHitNotifyHandling::Ignore;
}

bool APvPArenaCharacter::ResolveRangedCrosshairAim(FVector& OutAimOrigin, FVector& OutAimPoint) const
{
    FVector ViewLocation = GetActorLocation();
    FRotator ViewRotation = GetActorRotation();
    FVector ViewDirection = ViewRotation.Vector();

    if (const APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        int32 ViewportSizeX = 0;
        int32 ViewportSizeY = 0;
        FVector ScreenRayOrigin = FVector::ZeroVector;
        FVector ScreenRayDirection = FVector::ZeroVector;
        PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);

        if (ViewportSizeX > 0
            && ViewportSizeY > 0
            && PlayerController->DeprojectScreenPositionToWorld(
                static_cast<float>(ViewportSizeX) * 0.5f,
                static_cast<float>(ViewportSizeY) * 0.5f,
                ScreenRayOrigin,
                ScreenRayDirection))
        {
            ViewLocation = ScreenRayOrigin;
            ViewDirection = ScreenRayDirection.GetSafeNormal();
            ViewRotation = ViewDirection.Rotation();
        }
        else if (const APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager)
        {
            ViewLocation = CameraManager->GetCameraLocation();
            ViewRotation = CameraManager->GetCameraRotation();
            ViewDirection = ViewRotation.Vector();
        }
        else
        {
            PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
            ViewDirection = ViewRotation.Vector();
        }
    }
    else if (Controller)
    {
        Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
        ViewDirection = ViewRotation.Vector();
    }

    const float AimDistance = CombatComponent ? CombatComponent->GetRangedAimTraceDistance() : 2500.0f;
    const FVector AimEnd = ViewLocation + (ViewDirection * AimDistance);
    OutAimOrigin = ViewLocation;
    if (!GetWorld())
    {
        OutAimPoint = AimEnd;
        return true;
    }

    FCollisionQueryParams AimQueryParams(SCENE_QUERY_STAT(PvPArenaCharacterRangedAimTrace), false, this);
    AimQueryParams.AddIgnoredActor(this);

    FHitResult AimHitResult;
    GetWorld()->LineTraceSingleByChannel(
        AimHitResult,
        ViewLocation,
        AimEnd,
        ECC_Visibility,
        AimQueryParams);

    OutAimPoint = AimHitResult.bBlockingHit ? AimHitResult.ImpactPoint : AimEnd;
    return true;
}

void APvPArenaCharacter::CacheRangedAttackAim(const FVector& AimOrigin, const FVector& AimTarget)
{
    bHasCachedRangedAttackAim = true;
    CachedRangedAttackAimOrigin = AimOrigin;
    CachedRangedAttackAimTarget = AimTarget;
}

void APvPArenaCharacter::ClearCachedRangedAttackAim()
{
    bHasCachedRangedAttackAim = false;
    CachedRangedAttackAimOrigin = FVector::ZeroVector;
    CachedRangedAttackAimTarget = FVector::ZeroVector;
}

bool APvPArenaCharacter::CaptureCurrentRangedAttackAim()
{
    FVector AimOrigin = FVector::ZeroVector;
    FVector AimTarget = FVector::ZeroVector;
    if (!ResolveRangedCrosshairAim(AimOrigin, AimTarget))
    {
        UE_LOG(
            LogPvPArena,
            Warning,
            TEXT("CaptureCurrentRangedAttackAim failed Character=%s Role=%d Authority=%d Local=%d"),
            *GetName(),
            static_cast<int32>(GetLocalRole()),
            HasAuthority(),
            IsLocallyControlled());
        return false;
    }

    CacheRangedAttackAim(AimOrigin, AimTarget);
    UE_LOG(
        LogPvPArena,
        Log,
        TEXT("CaptureCurrentRangedAttackAim Character=%s Role=%d Authority=%d Local=%d AimOrigin=%s AimTarget=%s"),
        *GetName(),
        static_cast<int32>(GetLocalRole()),
        HasAuthority(),
        IsLocallyControlled(),
        *AimOrigin.ToCompactString(),
        *AimTarget.ToCompactString());
    return true;
}

void APvPArenaCharacter::RefreshRangedAttackAimFromCrosshair()
{
    if (!bRangedAttackInProgress || !bRangedChargeInputHeld)
    {
        return;
    }

    if (GetWorld() && !IsLocallyControlled())
    {
        return;
    }

    FVector AimPoint = FVector::ZeroVector;
    if (!ResolveRangedCrosshairAimPoint(AimPoint))
    {
        return;
    }

    const FVector AimDirection = AimPoint - GetActorLocation();
    if (AimDirection.IsNearlyZero())
    {
        return;
    }

    RangedAttackTargetYaw = FRotator::NormalizeAxis(AimDirection.Rotation().Yaw);
}

float APvPArenaCharacter::ResolveRangedAttackTargetYaw() const
{
    FVector AimPoint = FVector::ZeroVector;
    if (ResolveRangedCrosshairAimPoint(AimPoint))
    {
        const FVector AimDirection = AimPoint - GetActorLocation();
        if (!AimDirection.IsNearlyZero())
        {
            return FRotator::NormalizeAxis(AimDirection.Rotation().Yaw);
        }
    }

    return Controller ? Controller->GetControlRotation().Yaw : GetActorRotation().Yaw;
}

void APvPArenaCharacter::StartRangedAttackFacingLock(float TargetYaw)
{
    bRangedAttackFacingLocked = true;
    RangedAttackTargetYaw = FRotator::NormalizeAxis(TargetYaw);
}

void APvPArenaCharacter::UpdateRangedAttackFacing(float DeltaSeconds)
{
    if (!bRangedAttackInProgress || !bRangedAttackFacingLocked)
    {
        return;
    }

    const FRotator TargetRotation(0.0f, RangedAttackTargetYaw, 0.0f);
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaSeconds, RangedAttackTurnInterpSpeed));
}

void APvPArenaCharacter::ShowMeleeDebug(FVector Start, FVector End, bool bHit)
{
    if (HasAuthority())
    {
        MulticastDrawMeleeDebug(Start, End, bHit);
        return;
    }

    MulticastDrawMeleeDebug_Implementation(Start, End, bHit);
}

void APvPArenaCharacter::ApplyServerDamage(float Damage, AController* InstigatorController)
{
    if (bIsDead || bIsInvulnerable || Damage <= 0.0f)
    {
        return;
    }

    CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
    if (CurrentHealth <= 0.0f)
    {
        bIsDead = true;
        SetDeathInputSuppressed(true);
        PlayDeathAnimation();
        ForceNetUpdate();

        if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
        {
            MoveComp->DisableMovement();
        }

        if (APvPArenaGameMode* PvPGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<APvPArenaGameMode>() : nullptr)
        {
            PvPGameMode->HandlePlayerEliminated(Controller, InstigatorController);
        }
    }
}

void APvPArenaCharacter::SetInvulnerableForSeconds(float DurationSeconds)
{
    if (DurationSeconds <= 0.0f)
    {
        bIsInvulnerable = false;
        if (GetWorld())
        {
            GetWorldTimerManager().ClearTimer(InvulnerabilityTimerHandle);
        }
        return;
    }

    bIsInvulnerable = true;
    if (!GetWorld())
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(InvulnerabilityTimerHandle);
    GetWorldTimerManager().SetTimer(
        InvulnerabilityTimerHandle,
        [this]()
        {
            bIsInvulnerable = false;
        },
        DurationSeconds,
        false);
}

bool APvPArenaCharacter::PlayMeleeAttackMontage()
{
    if (!MeleeAttackMontage || !GetMesh())
    {
        return false;
    }

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance)
    {
        return false;
    }

    const float MontageDuration = PlayAnimMontage(MeleeAttackMontage, MeleeAttackPlayRate);
    if (MontageDuration <= 0.0f)
    {
        return false;
    }

    FOnMontageEnded MontageEndedDelegate;
    MontageEndedDelegate.BindUObject(this, &APvPArenaCharacter::HandleMeleeAttackMontageEnded);
    AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MeleeAttackMontage);
    PlayMeleeAttackEffect();
    PlayMeleeAttackSound();
    return true;
}

bool APvPArenaCharacter::PlayRangedAttackMontage(FName StartSectionName)
{
    if (!RangedAttackMontage || !GetMesh())
    {
        return false;
    }

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance)
    {
        return false;
    }

    const float MontageDuration = PlayAnimMontage(RangedAttackMontage, RangedAttackPlayRate);
    if (MontageDuration <= 0.0f)
    {
        return false;
    }

    FOnMontageEnded MontageEndedDelegate;
    MontageEndedDelegate.BindUObject(this, &APvPArenaCharacter::HandleRangedAttackMontageEnded);
    AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, RangedAttackMontage);
    if (!StartSectionName.IsNone())
    {
        AnimInstance->Montage_JumpToSection(StartSectionName, RangedAttackMontage);
    }
    return true;
}

void APvPArenaCharacter::PlayMeleeAttackEffect()
{
    if (!MeleeAttackEffect || !GetMesh() || GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    const FName AttachSocketName = GetMesh()->DoesSocketExist(MeleeAttackEffectSocketName)
        ? MeleeAttackEffectSocketName
        : NAME_None;

    UNiagaraFunctionLibrary::SpawnSystemAttached(
        MeleeAttackEffect,
        GetMesh(),
        AttachSocketName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        true);
}

void APvPArenaCharacter::PlayMeleeAttackSound()
{
    if (GetNetMode() == NM_DedicatedServer || !MeleeAttackAudioComponent || !MeleeAttackSound)
    {
        return;
    }

    MeleeAttackAudioComponent->Stop();
    MeleeAttackAudioComponent->Play(0.0f);

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    World->GetTimerManager().ClearTimer(MeleeAttackAudioTimerHandle);
    World->GetTimerManager().SetTimer(
        MeleeAttackAudioTimerHandle,
        this,
        &APvPArenaCharacter::StopMeleeAttackSound,
        MeleeAttackSoundDurationSeconds,
        false);
}

void APvPArenaCharacter::StopMeleeAttackSound()
{
    if (MeleeAttackAudioComponent)
    {
        MeleeAttackAudioComponent->Stop();
    }
}

void APvPArenaCharacter::PlayRangedAttackSound()
{
    if (GetNetMode() == NM_DedicatedServer || !RangedAttackAudioComponent || !RangedAttackSound)
    {
        return;
    }

    RangedAttackAudioComponent->Stop();
    RangedAttackAudioComponent->SetVolumeMultiplier(RangedAttackSoundVolume);
    RangedAttackAudioComponent->Play(0.0f);
}

void APvPArenaCharacter::HandleMeleeAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == MeleeAttackMontage || Montage == nullptr || bInterrupted)
    {
        FinishMeleeAttack();
    }
}

void APvPArenaCharacter::HandleRangedAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == RangedAttackMontage || Montage == nullptr || bInterrupted)
    {
        FinishRangedAttack();
    }
}

void APvPArenaCharacter::SetAttackMovementSuppressed(bool bSuppressed)
{
    if (bAttackMovementSuppressed == bSuppressed)
    {
        return;
    }

    bAttackMovementSuppressed = bSuppressed;

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        if (bSuppressed)
        {
            CachedMovementMode = MoveComp->MovementMode;
            MoveComp->StopMovementImmediately();

            if (HasAuthority() || IsLocallyControlled())
            {
                MoveComp->DisableMovement();
            }
        }
        else if (!bIsDead && MoveComp->MovementMode == MOVE_None)
        {
            MoveComp->SetMovementMode(static_cast<EMovementMode>(CachedMovementMode == MOVE_None ? MOVE_Walking : CachedMovementMode));
        }
    }

    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        PlayerController->SetIgnoreMoveInput(bSuppressed);
    }

    if (bSuppressed)
    {
        bSprintActive = false;
    }

    RefreshSprintMovementSpeed();
}

void APvPArenaCharacter::RefreshSprintMovementSpeed()
{
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp)
    {
        return;
    }

    if (BaseWalkSpeed <= 0.0f)
    {
        BaseWalkSpeed = MoveComp->MaxWalkSpeed;
    }

    MoveComp->MaxWalkSpeed = bSprintActive ? BaseWalkSpeed * SprintSpeedMultiplier : BaseWalkSpeed;
}

bool APvPArenaCharacter::CanSprint() const
{
    return !bIsDead && !bAttackMovementSuppressed && GetCharacterMovement() != nullptr;
}

void APvPArenaCharacter::ServerTryMeleeAttack_Implementation()
{
    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (!BeginMeleeAttack(Now))
    {
        return;
    }

    MulticastPlayMeleeAttackMontage();

    if (!PlayMeleeAttackMontage())
    {
        FinishMeleeAttack();
    }
}

void APvPArenaCharacter::ServerHandleMeleeAttackHitNotify_Implementation()
{
    TriggerMeleeAttackHit();
}

void APvPArenaCharacter::MulticastPlayMeleeAttackMontage_Implementation()
{
    if (HasAuthority())
    {
        return;
    }

    bMeleeAttackInProgress = true;
    bMeleeAttackHitTriggered = false;
    SetAttackMovementSuppressed(true);
    PlayMeleeAttackMontage();
}

void APvPArenaCharacter::ServerHandleRangedAttackHitNotify_Implementation(bool bHasAim, FVector_NetQuantize AimOrigin, FVector_NetQuantize AimTarget)
{
    UE_LOG(
        LogPvPArena,
        Log,
        TEXT("ServerHandleRangedAttackHitNotify Character=%s Role=%d Authority=%d InProgress=%d ReleaseCommitted=%d HitTriggered=%d HasAim=%d AimOrigin=%s AimTarget=%s"),
        *GetName(),
        static_cast<int32>(GetLocalRole()),
        HasAuthority(),
        bRangedAttackInProgress,
        bRangedReleaseCommitted,
        bRangedAttackHitTriggered,
        bHasAim,
        *AimOrigin.ToCompactString(),
        *AimTarget.ToCompactString());
    if (bHasAim && !bHasCachedRangedAttackAim)
    {
        CacheRangedAttackAim(AimOrigin, AimTarget);
    }
    else if (bHasAim && bHasCachedRangedAttackAim)
    {
        UE_LOG(
            LogPvPArena,
            Log,
            TEXT("ServerHandleRangedAttackHitNotify preserving cached release aim Character=%s CachedAimOrigin=%s CachedAimTarget=%s LateAimOrigin=%s LateAimTarget=%s"),
            *GetName(),
            *CachedRangedAttackAimOrigin.ToCompactString(),
            *CachedRangedAttackAimTarget.ToCompactString(),
            *AimOrigin.ToCompactString(),
            *AimTarget.ToCompactString());
    }
    TriggerRangedAttackHit();
}

void APvPArenaCharacter::ServerSetSprintInputHeld_Implementation(bool bNewSprintInputHeld)
{
    bSprintInputHeld = bNewSprintInputHeld;
    if (!bSprintInputHeld)
    {
        bSprintActive = false;
        RefreshSprintMovementSpeed();
    }
}

void APvPArenaCharacter::MulticastPlayRangedAttackMontage_Implementation(float TargetYaw)
{
    if (HasAuthority())
    {
        return;
    }

    bRangedAttackInProgress = true;
    bRangedChargeInputHeld = true;
    bRangedReleaseCommitted = false;
    bRangedAttackHitTriggered = false;
    StartRangedAttackFacingLock(TargetYaw);
    SetAttackMovementSuppressed(true);
    UE_LOG(
        LogPvPArena,
        Log,
        TEXT("MulticastPlayRangedAttackMontage Character=%s Role=%d Local=%d TargetYaw=%.2f"),
        *GetName(),
        static_cast<int32>(GetLocalRole()),
        IsLocallyControlled(),
        TargetYaw);
    PlayRangedAttackMontage(RangedAttackStartSectionName);
}

void APvPArenaCharacter::MulticastResolveRangedCharge_Implementation(bool bCommitted)
{
    if (HasAuthority())
    {
        return;
    }

    bRangedChargeInputHeld = false;
    bRangedReleaseCommitted = bCommitted;
    UE_LOG(
        LogPvPArena,
        Log,
        TEXT("MulticastResolveRangedCharge Character=%s Role=%d Local=%d Committed=%d CachedAim=%d AimOrigin=%s AimTarget=%s"),
        *GetName(),
        static_cast<int32>(GetLocalRole()),
        IsLocallyControlled(),
        bCommitted,
        bHasCachedRangedAttackAim,
        *CachedRangedAttackAimOrigin.ToCompactString(),
        *CachedRangedAttackAimTarget.ToCompactString());
    const FName TargetSectionName = bCommitted ? RangedAttackReleaseSectionName : RangedAttackCancelSectionName;
    if (!JumpToRangedMontageSection(TargetSectionName))
    {
        PlayRangedAttackMontage(TargetSectionName);
    }
}

void APvPArenaCharacter::MulticastDrawMeleeDebug_Implementation(FVector Start, FVector End, bool bHit)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const float DebugDuration = 1.0f;
    const float DebugRadius = 70.0f;
    const FColor DebugColor = bHit ? FColor::Red : FColor::Green;
    DrawDebugCapsule(
        World,
        (Start + End) * 0.5f,
        140.0f * 0.5f,
        DebugRadius,
        FRotationMatrix::MakeFromZ((End - Start).GetSafeNormal()).ToQuat(),
        DebugColor,
        false,
        DebugDuration,
        0,
        1.5f);
    DrawDebugSphere(World, End, DebugRadius, 16, DebugColor, false, DebugDuration, 0, 1.0f);
}

void APvPArenaCharacter::MulticastPlayRangedAttackSound_Implementation()
{
    PlayRangedAttackSound();
}

void APvPArenaCharacter::ServerTryRangedAttack_Implementation()
{
    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    UE_LOG(
        LogPvPArena,
        Log,
        TEXT("ServerTryRangedAttack Character=%s Role=%d Authority=%d"),
        *GetName(),
        static_cast<int32>(GetLocalRole()),
        HasAuthority());
    if (!BeginRangedAttack(Now))
    {
        return;
    }

    MulticastPlayRangedAttackMontage(RangedAttackTargetYaw);
    MulticastResolveRangedCharge(true);

    if (!PlayRangedAttackMontage(RangedAttackReleaseSectionName))
    {
        FinishRangedAttack();
    }
}

void APvPArenaCharacter::ServerBeginRangedCharge_Implementation()
{
    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    UE_LOG(
        LogPvPArena,
        Log,
        TEXT("ServerBeginRangedCharge Character=%s Role=%d Authority=%d"),
        *GetName(),
        static_cast<int32>(GetLocalRole()),
        HasAuthority());
    if (!BeginRangedCharge(Now))
    {
        return;
    }

    MulticastPlayRangedAttackMontage(RangedAttackTargetYaw);

    if (!PlayRangedAttackMontage(RangedAttackStartSectionName))
    {
        FinishRangedAttack();
    }
}

void APvPArenaCharacter::ServerReleaseRangedCharge_Implementation()
{
    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    UE_LOG(
        LogPvPArena,
        Log,
        TEXT("ServerReleaseRangedCharge Character=%s Role=%d Authority=%d"),
        *GetName(),
        static_cast<int32>(GetLocalRole()),
        HasAuthority());
    const bool bCommitted = ReleaseRangedCharge(Now);
    if (!bRangedAttackInProgress)
    {
        return;
    }

    MulticastResolveRangedCharge(bCommitted);
}

void APvPArenaCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APvPArenaCharacter, CurrentHealth);
    DOREPLIFETIME(APvPArenaCharacter, bIsDead);
    DOREPLIFETIME(APvPArenaCharacter, bIsInvulnerable);
}
