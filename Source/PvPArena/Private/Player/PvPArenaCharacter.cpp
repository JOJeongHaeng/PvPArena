#include "Player/PvPArenaCharacter.h"

#include "Blueprint/UserWidget.h"
#include "Combat/PvPCombatComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Game/PvPArenaGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "UI/PvPArenaHUDWidget.h"

APvPArenaCharacter::APvPArenaCharacter()
{
    bReplicates = true;
    CombatComponent = CreateDefaultSubobject<UPvPCombatComponent>(TEXT("CombatComponent"));
    HUDWidgetClass = UPvPArenaHUDWidget::StaticClass();
}

void APvPArenaCharacter::BeginPlay()
{
    Super::BeginPlay();
    TryApplyInputMappingContext();
}

void APvPArenaCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    TryApplyInputMappingContext();
}

void APvPArenaCharacter::OnRep_Controller()
{
    Super::OnRep_Controller();
    TryApplyInputMappingContext();
}

void APvPArenaCharacter::OnRep_CurrentHealth()
{
    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
}

void APvPArenaCharacter::OnRep_IsDead()
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

void APvPArenaCharacter::TryCreateHUDWidget()
{
    if (ActiveHUDWidget || !IsLocallyControlled())
    {
        return;
    }

    TSubclassOf<UUserWidget> WidgetClassToCreate = HUDWidgetClass;
    if (!WidgetClassToCreate)
    {
        WidgetClassToCreate = UPvPArenaHUDWidget::StaticClass();
    }
    if (!WidgetClassToCreate)
    {
        return;
    }

    APlayerController* PlayerController = Cast<APlayerController>(Controller);
    if (!PlayerController || !PlayerController->IsLocalController())
    {
        RetryCreateHUDWidget();
        return;
    }

    UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(PlayerController, WidgetClassToCreate);
    if (!CreatedWidget)
    {
        RetryCreateHUDWidget();
        return;
    }

    GetWorldTimerManager().ClearTimer(HUDRetryTimerHandle);
    CreatedWidget->AddToViewport();
    ActiveHUDWidget = CreatedWidget;
}

void APvPArenaCharacter::RetryCreateHUDWidget()
{
    if (!GetWorld() || HUDRetryTimerHandle.IsValid())
    {
        return;
    }

    GetWorldTimerManager().SetTimer(
        HUDRetryTimerHandle,
        this,
        &APvPArenaCharacter::TryCreateHUDWidget,
        0.2f,
        true);
}

void APvPArenaCharacter::ApplyServerDamage(float Damage, AController* InstigatorController)
{
    if (bIsDead || Damage <= 0.0f)
    {
        return;
    }

    CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
    if (CurrentHealth <= 0.0f)
    {
        bIsDead = true;

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

void APvPArenaCharacter::ServerTryMeleeAttack_Implementation()
{
    if (!CombatComponent || bIsDead)
    {
        return;
    }

    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (!CombatComponent->CanUseMelee(Now))
    {
        return;
    }

    CombatComponent->TryServerMeleeAttack(this);
    CombatComponent->MarkMeleeUsed(Now);
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
}
