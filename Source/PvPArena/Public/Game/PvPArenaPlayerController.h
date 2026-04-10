#pragma once

#include "CoreMinimal.h"
#include "Game/PvPArenaPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "PvPArenaPlayerController.generated.h"

class UUserWidget;

UCLASS()
class PVPARENA_API APvPArenaPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    static constexpr float FreeSpectatorMoveSpeed = 1200.0f;
    static constexpr float FreeSpectatorLookSensitivity = 1.25f;

    APvPArenaPlayerController();
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void PlayerTick(float DeltaTime) override;
    virtual void SetupInputComponent() override;
    static FRotator BuildFreeSpectatorControlRotation(const FRotator& CurrentRotation, float MouseDeltaX, float MouseDeltaY);
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void RequestLobbyMatchStart();

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void ToggleLobbyReady();

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void SubmitLobbyNickname(const FString& Nickname);
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void RequestLobbyMatchModeChange(EPvPALobbyMatchMode NewLobbyMatchMode);

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void RequestLobbyTeamSelection(EPvPALobbyTeam NewLobbyTeam);

    UFUNCTION(Client, Reliable)
    void ClientEnterFreeSpectatorMode();
    void ClientEnterFreeSpectatorMode_Implementation();

    UFUNCTION()
    void PrepareForRoundRestart();

    UFUNCTION(Client, Reliable)
    void ClientPrepareForRoundRestart();
    void ClientPrepareForRoundRestart_Implementation();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> HUDWidgetClass;

private:
    void ApplyRoundRestartPreparation();
    void HandleToggleSettingsMenu();
    void EnterFreeSpectatorMode();
    void RetryAttachSpectatorViewTarget();
    void UpdateFreeSpectator(float DeltaTime);

    UFUNCTION(Server, Reliable)
    void ServerRequestLobbyMatchStart();

    UFUNCTION(Server, Reliable)
    void ServerSetLobbyReady(bool bReadyForStart);

    UFUNCTION(Server, Reliable)
    void ServerSubmitLobbyNickname(const FString& Nickname);

    UFUNCTION(Server, Reliable)
    void ServerRequestLobbyMatchModeChange(EPvPALobbyMatchMode NewLobbyMatchMode);
    void ServerRequestLobbyMatchModeChange_Implementation(EPvPALobbyMatchMode NewLobbyMatchMode);

    UFUNCTION(Server, Reliable)
    void ServerRequestLobbyTeamSelection(EPvPALobbyTeam NewLobbyTeam);
    void ServerRequestLobbyTeamSelection_Implementation(EPvPALobbyTeam NewLobbyTeam);

    void TryCreateHUDWidget();
    void RetryCreateHUDWidget();

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> ActiveHUDWidget;

    FTimerHandle HUDRetryTimerHandle;
    FTimerHandle SpectatorViewRetryTimerHandle;
};
