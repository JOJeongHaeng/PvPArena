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

protected:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> HUDWidgetClass;

private:
    void TryCreateHUDWidget();
    void RetryCreateHUDWidget();

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> ActiveHUDWidget;

    FTimerHandle HUDRetryTimerHandle;
};
