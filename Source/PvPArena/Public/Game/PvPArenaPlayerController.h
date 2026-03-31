#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PvPArenaPlayerController.generated.h"

class UUserWidget;

UCLASS()
class PVPARENA_API APvPArenaPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    APvPArenaPlayerController();
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void SetupInputComponent() override;
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void RequestLobbyMatchStart();

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void ToggleLobbyReady();

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void SubmitLobbyNickname(const FString& Nickname);

protected:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> HUDWidgetClass;

private:
    void HandleToggleSettingsMenu();

    UFUNCTION(Server, Reliable)
    void ServerRequestLobbyMatchStart();

    UFUNCTION(Server, Reliable)
    void ServerSetLobbyReady(bool bReadyForStart);

    UFUNCTION(Server, Reliable)
    void ServerSubmitLobbyNickname(const FString& Nickname);

    void TryCreateHUDWidget();
    void RetryCreateHUDWidget();

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> ActiveHUDWidget;

    FTimerHandle HUDRetryTimerHandle;
};
