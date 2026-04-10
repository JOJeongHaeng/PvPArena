#include "Game/PvPArenaPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "Game/PvPArenaGameMode.h"
#include "Game/PvPArenaPlayerState.h"
#include "GameFramework/SpectatorPawn.h"
#include "GameFramework/PlayerState.h"
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
    ApplyRoundRestartPreparation();
    if (IsLocalController() && InPawn)
    {
        SetViewTarget(InPawn);
    }
    TryCreateHUDWidget();
}

void APvPArenaPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);
    UpdateFreeSpectator(DeltaTime);
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

void APvPArenaPlayerController::RequestLobbyMatchModeChange(EPvPALobbyMatchMode NewLobbyMatchMode)
{
    if (HasAuthority())
    {
        ServerRequestLobbyMatchModeChange_Implementation(NewLobbyMatchMode);
        return;
    }

    ServerRequestLobbyMatchModeChange(NewLobbyMatchMode);
}

void APvPArenaPlayerController::RequestLobbyTeamSelection(EPvPALobbyTeam NewLobbyTeam)
{
    if (HasAuthority())
    {
        ServerRequestLobbyTeamSelection_Implementation(NewLobbyTeam);
        return;
    }

    ServerRequestLobbyTeamSelection(NewLobbyTeam);
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

void APvPArenaPlayerController::ServerRequestLobbyMatchModeChange_Implementation(EPvPALobbyMatchMode NewLobbyMatchMode)
{
    APvPArenaGameMode* PvPGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<APvPArenaGameMode>() : nullptr;
    if (!PvPGameMode)
    {
        return;
    }

    PvPGameMode->HandleLobbyMatchModeChanged(this, NewLobbyMatchMode);
}

void APvPArenaPlayerController::ServerRequestLobbyTeamSelection_Implementation(EPvPALobbyTeam NewLobbyTeam)
{
    APvPArenaPlayerState* PvPPlayerState = GetPlayerState<APvPArenaPlayerState>();
    APvPArenaGameMode* PvPGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<APvPArenaGameMode>() : nullptr;
    if (!PvPPlayerState || !PvPGameMode)
    {
        return;
    }

    PvPGameMode->HandleLobbyTeamSelectionChanged(PvPPlayerState, NewLobbyTeam);
}

void APvPArenaPlayerController::ClientEnterFreeSpectatorMode_Implementation()
{
    EnterFreeSpectatorMode();
}

void APvPArenaPlayerController::ApplyRoundRestartPreparation()
{
    SetIgnoreMoveInput(false);
    SetIgnoreLookInput(false);

    if (GetWorld())
    {
        GetWorldTimerManager().ClearTimer(SpectatorViewRetryTimerHandle);
    }
}

void APvPArenaPlayerController::PrepareForRoundRestart()
{
    ApplyRoundRestartPreparation();

    if (!GetWorld())
    {
        return;
    }

    if (PlayerState)
    {
        PlayerState->SetIsSpectator(false);
        PlayerState->SetIsOnlyASpectator(false);
    }

    ChangeState(NAME_Playing);
    ClientGotoState(NAME_Playing);
    ClientPrepareForRoundRestart();
    GetWorldTimerManager().ClearTimer(SpectatorViewRetryTimerHandle);
}

void APvPArenaPlayerController::ClientPrepareForRoundRestart_Implementation()
{
    ApplyRoundRestartPreparation();
    if (PlayerState)
    {
        PlayerState->SetIsSpectator(false);
        PlayerState->SetIsOnlyASpectator(false);
    }
    ChangeState(NAME_Playing);

    if (APawn* ControlledPawn = GetPawn())
    {
        SetViewTarget(ControlledPawn);
    }
}

FRotator APvPArenaPlayerController::BuildFreeSpectatorControlRotation(
    const FRotator& CurrentRotation,
    float MouseDeltaX,
    float MouseDeltaY)
{
    FRotator NextRotation = CurrentRotation;
    NextRotation.Yaw += (MouseDeltaX * FreeSpectatorLookSensitivity);
    NextRotation.Pitch = FMath::ClampAngle(
        CurrentRotation.Pitch + (MouseDeltaY * FreeSpectatorLookSensitivity),
        -89.0f,
        89.0f);
    NextRotation.Roll = 0.0f;
    return NextRotation;
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

void APvPArenaPlayerController::EnterFreeSpectatorMode()
{
    SetIgnoreMoveInput(false);
    SetIgnoreLookInput(false);

    if (!GetWorld())
    {
        return;
    }

    StartSpectatingOnly();
    ChangeState(NAME_Spectating);
    ClientGotoState(NAME_Spectating);
    RetryAttachSpectatorViewTarget();
}

void APvPArenaPlayerController::RetryAttachSpectatorViewTarget()
{
    if (!IsLocalController() || !GetWorld())
    {
        return;
    }

    if (ASpectatorPawn* SpawnedSpectatorPawn = GetSpectatorPawn())
    {
        if (GetViewTarget() != SpawnedSpectatorPawn)
        {
            SetViewTarget(SpawnedSpectatorPawn);
        }

        if (GetViewTarget() == SpawnedSpectatorPawn)
        {
            GetWorldTimerManager().ClearTimer(SpectatorViewRetryTimerHandle);
            return;
        }
    }

    if (!SpectatorViewRetryTimerHandle.IsValid())
    {
        GetWorldTimerManager().SetTimer(
            SpectatorViewRetryTimerHandle,
            this,
            &APvPArenaPlayerController::RetryAttachSpectatorViewTarget,
            0.1f,
            true);
    }
}

void APvPArenaPlayerController::UpdateFreeSpectator(float DeltaTime)
{
    if (!IsLocalController() || !IsInState(NAME_Spectating))
    {
        return;
    }

    ASpectatorPawn* FreeSpectatorPawn = GetSpectatorPawn();
    if (!FreeSpectatorPawn)
    {
        return;
    }

    if (GetViewTarget() != FreeSpectatorPawn)
    {
        SetViewTarget(FreeSpectatorPawn);
        if (GetViewTarget() != FreeSpectatorPawn)
        {
            return;
        }
    }

    float MouseDeltaX = 0.0f;
    float MouseDeltaY = 0.0f;
    GetInputMouseDelta(MouseDeltaX, MouseDeltaY);

    FRotator CurrentControlRotation = GetControlRotation();
    if (!FMath::IsNearlyZero(MouseDeltaX) || !FMath::IsNearlyZero(MouseDeltaY))
    {
        CurrentControlRotation = BuildFreeSpectatorControlRotation(CurrentControlRotation, MouseDeltaX, MouseDeltaY);
        SetControlRotation(CurrentControlRotation);
    }

    const FRotator YawOnlyRotation(0.0f, CurrentControlRotation.Yaw, 0.0f);
    const FVector ForwardDirection = FRotationMatrix(YawOnlyRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawOnlyRotation).GetUnitAxis(EAxis::Y);

    float ForwardValue = 0.0f;
    ForwardValue += IsInputKeyDown(EKeys::W) ? 1.0f : 0.0f;
    ForwardValue -= IsInputKeyDown(EKeys::S) ? 1.0f : 0.0f;

    float RightValue = 0.0f;
    RightValue += IsInputKeyDown(EKeys::D) ? 1.0f : 0.0f;
    RightValue -= IsInputKeyDown(EKeys::A) ? 1.0f : 0.0f;

    float VerticalValue = 0.0f;
    VerticalValue += IsInputKeyDown(EKeys::E) ? 1.0f : 0.0f;
    VerticalValue -= IsInputKeyDown(EKeys::Q) ? 1.0f : 0.0f;

    FVector MovementDirection = FVector::ZeroVector;
    MovementDirection += ForwardDirection * ForwardValue;
    MovementDirection += RightDirection * RightValue;
    MovementDirection += FVector::UpVector * VerticalValue;

    if (!MovementDirection.IsNearlyZero())
    {
        FreeSpectatorPawn->SetActorLocation(
            FreeSpectatorPawn->GetActorLocation() + (MovementDirection.GetClampedToMaxSize(1.0f) * FreeSpectatorMoveSpeed * DeltaTime));
    }

    FreeSpectatorPawn->SetActorRotation(CurrentControlRotation);
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
