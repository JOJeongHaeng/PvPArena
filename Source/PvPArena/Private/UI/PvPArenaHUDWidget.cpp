#include "UI/PvPArenaHUDWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Game/PvPArenaGameState.h"
#include "Game/PvPArenaPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Player/PvPArenaCharacter.h"
#include "Blueprint/WidgetTree.h"

void UPvPArenaHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTree();
    RefreshWidgetData();
}

void UPvPArenaHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    RefreshWidgetData();
}

void UPvPArenaHUDWidget::BuildWidgetTree()
{
    if (!WidgetTree)
    {
        return;
    }

    RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
    WidgetTree->RootWidget = RootCanvas;

    HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
    HealthText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HealthText"));
    ScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ScoreText"));
    TimerText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TimerText"));
    RoundStateText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RoundStateText"));

    if (!RootCanvas || !HealthBar || !HealthText || !ScoreText || !TimerText || !RoundStateText)
    {
        return;
    }

    HealthBar->SetPercent(1.0f);
    HealthBar->SetFillColorAndOpacity(FLinearColor(0.9f, 0.2f, 0.2f, 1.0f));

    UCanvasPanelSlot* HealthBarSlot = RootCanvas->AddChildToCanvas(HealthBar);
    UCanvasPanelSlot* HealthTextSlot = RootCanvas->AddChildToCanvas(HealthText);
    UCanvasPanelSlot* ScoreTextSlot = RootCanvas->AddChildToCanvas(ScoreText);
    UCanvasPanelSlot* TimerTextSlot = RootCanvas->AddChildToCanvas(TimerText);
    UCanvasPanelSlot* RoundStateTextSlot = RootCanvas->AddChildToCanvas(RoundStateText);

    if (HealthBarSlot)
    {
        HealthBarSlot->SetAutoSize(false);
        HealthBarSlot->SetPosition(FVector2D(40.0f, 40.0f));
        HealthBarSlot->SetSize(FVector2D(280.0f, 18.0f));
    }

    if (HealthTextSlot)
    {
        HealthTextSlot->SetAutoSize(true);
        HealthTextSlot->SetPosition(FVector2D(40.0f, 62.0f));
    }

    if (ScoreTextSlot)
    {
        ScoreTextSlot->SetAutoSize(true);
        ScoreTextSlot->SetPosition(FVector2D(40.0f, 86.0f));
    }

    if (TimerTextSlot)
    {
        TimerTextSlot->SetAutoSize(true);
        TimerTextSlot->SetPosition(FVector2D(40.0f, 110.0f));
    }

    if (RoundStateTextSlot)
    {
        RoundStateTextSlot->SetAutoSize(true);
        RoundStateTextSlot->SetPosition(FVector2D(40.0f, 134.0f));
    }
}

void UPvPArenaHUDWidget::RefreshWidgetData()
{
    APlayerController* PlayerController = GetOwningPlayer();
    if (!PlayerController)
    {
        return;
    }

    if (const APvPArenaCharacter* Character = Cast<APvPArenaCharacter>(PlayerController->GetPawn()))
    {
        const float MaxHp = FMath::Max(1.0f, Character->GetMaxHealth());
        const float CurrentHp = Character->GetCurrentHealth();
        if (HealthBar)
        {
            HealthBar->SetPercent(CurrentHp / MaxHp);
        }

        if (HealthText)
        {
            HealthText->SetText(FText::FromString(FString::Printf(TEXT("HP: %.0f / %.0f"), CurrentHp, MaxHp)));
        }
    }

    if (const APvPArenaPlayerState* PvPPlayerState = PlayerController->GetPlayerState<APvPArenaPlayerState>())
    {
        if (ScoreText)
        {
            ScoreText->SetText(FText::FromString(FString::Printf(TEXT("K / D: %d / %d"), PvPPlayerState->GetKills(), PvPPlayerState->GetDeaths())));
        }
    }

    if (const APvPArenaGameState* PvPGameState = GetWorld() ? GetWorld()->GetGameState<APvPArenaGameState>() : nullptr)
    {
        if (TimerText)
        {
            TimerText->SetText(FText::FromString(FString::Printf(TEXT("Time: %d"), PvPGameState->GetRemainingRoundTimeSeconds())));
        }

        if (RoundStateText)
        {
            RoundStateText->SetText(FText::FromString(FString::Printf(TEXT("Round: %s"), *RoundStateToString(static_cast<uint8>(PvPGameState->GetRoundState())))));
        }
    }
}

FString UPvPArenaHUDWidget::RoundStateToString(uint8 RoundStateValue)
{
    switch (static_cast<EPvPARoundState>(RoundStateValue))
    {
    case EPvPARoundState::Playing:
        return TEXT("Playing");
    case EPvPARoundState::RoundEnd:
        return TEXT("RoundEnd");
    case EPvPARoundState::SuddenDeath:
        return TEXT("SuddenDeath");
    default:
        return TEXT("Unknown");
    }
}
