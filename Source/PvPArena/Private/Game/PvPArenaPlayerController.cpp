#include "Game/PvPArenaPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "Game/PvPArenaGameMode.h"
#include "Game/PvPArenaPlayerState.h"
#include "InputCoreTypes.h"
#include "TimerManager.h"
#include "UI/PvPArenaHUDWidget.h"

APvPArenaPlayerController::APvPArenaPlayerController()
{
    HUDWidgetClass = UPvPArenaHUDWidget::StaticClass();
}

void APvPArenaPlayerController::BeginPlay()
{
    Super::BeginPlay();
    TryCreateHUDWidget();
}

void APvPArenaPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    TryCreateHUDWidget();
}

void APvPArenaPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (!InputComponent)
    {
        return;
    }

    InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &APvPArenaPlayerController::HandleToggleSettingsMenu);
}

void APvPArenaPlayerController::RequestLobbyMatchStart()
{
    if (HasAuthority())
    {
        ServerRequestLobbyMatchStart_Implementation();
        return;
    }

    ServerRequestLobbyMatchStart();
}

void APvPArenaPlayerController::ToggleLobbyReady()
{
    const APvPArenaPlayerState* PvPPlayerState = GetPlayerState<APvPArenaPlayerState>();
    const bool bNewReady = !(PvPPlayerState && PvPPlayerState->IsReadyForLobbyStart());

    if (HasAuthority())
    {
        ServerSetLobbyReady_Implementation(bNewReady);
        return;
    }

    ServerSetLobbyReady(bNewReady);
}

void APvPArenaPlayerController::SubmitLobbyNickname(const FString& Nickname)
{
    if (HasAuthority())
    {
        ServerSubmitLobbyNickname_Implementation(Nickname);
        return;
    }

    ServerSubmitLobbyNickname(Nickname);
}

void APvPArenaPlayerController::ServerRequestLobbyMatchStart_Implementation()
{
    APvPArenaGameMode* PvPGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<APvPArenaGameMode>() : nullptr;
    if (!PvPGameMode)
    {
        return;
    }

    PvPGameMode->RequestLobbyMatchStart(this);
}

void APvPArenaPlayerController::ServerSetLobbyReady_Implementation(bool bReadyForStart)
{
    APvPArenaPlayerState* PvPPlayerState = GetPlayerState<APvPArenaPlayerState>();
    APvPArenaGameMode* PvPGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<APvPArenaGameMode>() : nullptr;
    if (!PvPPlayerState || !PvPGameMode)
    {
        return;
    }

    PvPGameMode->HandleLobbyReadyStateChanged(PvPPlayerState, bReadyForStart);
}

void APvPArenaPlayerController::ServerSubmitLobbyNickname_Implementation(const FString& Nickname)
{
    APvPArenaPlayerState* PvPPlayerState = GetPlayerState<APvPArenaPlayerState>();
    APvPArenaGameMode* PvPGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<APvPArenaGameMode>() : nullptr;
    if (!PvPPlayerState || !PvPGameMode)
    {
        return;
    }

    PvPGameMode->HandleLobbyDisplayNicknameChanged(PvPPlayerState, Nickname);
}

void APvPArenaPlayerController::HandleToggleSettingsMenu()
{
    UPvPArenaHUDWidget* PvPHUDWidget = Cast<UPvPArenaHUDWidget>(ActiveHUDWidget);
    if (!PvPHUDWidget)
    {
        return;
    }

    PvPHUDWidget->ToggleSettingsMenu();
}

void APvPArenaPlayerController::TryCreateHUDWidget()
{
    if (ActiveHUDWidget || !IsLocalController())
    {
        return;
    }

    TSubclassOf<UUserWidget> WidgetClassToCreate = HUDWidgetClass;
    if (!WidgetClassToCreate || !WidgetClassToCreate->IsChildOf(UPvPArenaHUDWidget::StaticClass()))
    {
        WidgetClassToCreate = UPvPArenaHUDWidget::StaticClass();
    }

    if (!WidgetClassToCreate)
    {
        return;
    }

    UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(this, WidgetClassToCreate);
    if (!CreatedWidget)
    {
        RetryCreateHUDWidget();
        return;
    }

    GetWorldTimerManager().ClearTimer(HUDRetryTimerHandle);
    CreatedWidget->AddToViewport(100);
    ActiveHUDWidget = CreatedWidget;
}

void APvPArenaPlayerController::RetryCreateHUDWidget()
{
    if (!GetWorld() || HUDRetryTimerHandle.IsValid())
    {
        return;
    }

    GetWorldTimerManager().SetTimer(
        HUDRetryTimerHandle,
        this,
        &APvPArenaPlayerController::TryCreateHUDWidget,
        0.2f,
        true);
}
