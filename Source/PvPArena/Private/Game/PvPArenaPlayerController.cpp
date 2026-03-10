#include "Game/PvPArenaPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
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
