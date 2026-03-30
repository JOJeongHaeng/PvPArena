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

protected:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> HUDWidgetClass;

private:
    void HandleToggleSettingsMenu();

    UFUNCTION(Server, Reliable)
    void ServerRequestLobbyMatchStart();

    UFUNCTION(Server, Reliable)
    void ServerSetLobbyReady(bool bReadyForStart);

    void TryCreateHUDWidget();
    void RetryCreateHUDWidget();

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> ActiveHUDWidget;

    FTimerHandle HUDRetryTimerHandle;
};
