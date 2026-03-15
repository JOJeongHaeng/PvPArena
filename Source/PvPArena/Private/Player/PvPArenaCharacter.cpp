#include "Player/PvPArenaCharacter.h"

#include "Animation/AnimationAsset.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Combat/PvPCombatComponent.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Game/PvPArenaGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

APvPArenaCharacter::APvPArenaCharacter()
{
    bReplicates = true;
    CombatComponent = CreateDefaultSubobject<UPvPCombatComponent>(TEXT("CombatComponent"));

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
}

void APvPArenaCharacter::BeginPlay()
{
    Super::BeginPlay();
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
}

float APvPArenaCharacter::GetDeathAnimationDuration() const
{
    return DeathAnimation ? DeathAnimation->GetPlayLength() : 0.0f;
}

bool APvPArenaCharacter::BeginMeleeAttack(float NowSeconds)
{
    if (!CombatComponent || bIsDead || bMeleeAttackInProgress || !CombatComponent->CanUseMelee(NowSeconds))
    {
        return false;
    }

    CombatComponent->MarkMeleeUsed(NowSeconds);
    bMeleeAttackInProgress = true;
    bMeleeAttackHitTriggered = false;
    SetMeleeMovementSuppressed(true);
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
    SetMeleeMovementSuppressed(false);
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
    return true;
}

void APvPArenaCharacter::HandleMeleeAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == MeleeAttackMontage || Montage == nullptr || bInterrupted)
    {
        FinishMeleeAttack();
    }
}

void APvPArenaCharacter::SetMeleeMovementSuppressed(bool bSuppressed)
{
    if (bMeleeMovementSuppressed == bSuppressed)
    {
        return;
    }

    bMeleeMovementSuppressed = bSuppressed;

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
    SetMeleeMovementSuppressed(true);
    PlayMeleeAttackMontage();
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

void APvPArenaCharacter::ServerTryRangedAttack_Implementation()
{
    if (!CombatComponent || bIsDead)
    {
        return;
    }

    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (!CombatComponent->CanUseRanged(Now))
    {
        return;
    }

    CombatComponent->TryServerRangedAttack(this);
    CombatComponent->MarkRangedUsed(Now);
}

void APvPArenaCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APvPArenaCharacter, CurrentHealth);
    DOREPLIFETIME(APvPArenaCharacter, bIsDead);
    DOREPLIFETIME(APvPArenaCharacter, bIsInvulnerable);
}
