#include "UI/PvPArenaHUDWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Game/PvPArenaGameState.h"
#include "Game/PvPArenaPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Player/PvPArenaCharacter.h"
#include "TimerManager.h"
#include "Blueprint/WidgetTree.h"

void UPvPArenaHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTree();
    RefreshWidgetData();
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(RefreshTimerHandle, this, &UPvPArenaHUDWidget::RefreshWidgetData, 0.15f, true);
    }
}

void UPvPArenaHUDWidget::NativeDestruct()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RefreshTimerHandle);
    }

    Super::NativeDestruct();
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
    RoundScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RoundScoreText"));
    MatchScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MatchScoreText"));
    TimerText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TimerText"));
    RoundStateText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RoundStateText"));
    ResultText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ResultText"));
    NextRoundText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NextRoundText"));

    if (!RootCanvas || !HealthBar || !HealthText || !RoundScoreText || !MatchScoreText || !TimerText || !RoundStateText || !ResultText || !NextRoundText)
    {
        return;
    }

    HealthBar->SetPercent(1.0f);
    HealthBar->SetFillColorAndOpacity(FLinearColor(0.9f, 0.2f, 0.2f, 1.0f));

    UCanvasPanelSlot* HealthBarSlot = RootCanvas->AddChildToCanvas(HealthBar);
    UCanvasPanelSlot* HealthTextSlot = RootCanvas->AddChildToCanvas(HealthText);
    UCanvasPanelSlot* RoundScoreTextSlot = RootCanvas->AddChildToCanvas(RoundScoreText);
    UCanvasPanelSlot* MatchScoreTextSlot = RootCanvas->AddChildToCanvas(MatchScoreText);
    UCanvasPanelSlot* TimerTextSlot = RootCanvas->AddChildToCanvas(TimerText);
    UCanvasPanelSlot* RoundStateTextSlot = RootCanvas->AddChildToCanvas(RoundStateText);
    UCanvasPanelSlot* ResultTextSlot = RootCanvas->AddChildToCanvas(ResultText);
    UCanvasPanelSlot* NextRoundTextSlot = RootCanvas->AddChildToCanvas(NextRoundText);

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

    if (RoundScoreTextSlot)
    {
        RoundScoreTextSlot->SetAutoSize(true);
        RoundScoreTextSlot->SetPosition(FVector2D(40.0f, 86.0f));
    }

    if (MatchScoreTextSlot)
    {
        MatchScoreTextSlot->SetAutoSize(true);
        MatchScoreTextSlot->SetPosition(FVector2D(40.0f, 110.0f));
    }

    if (TimerTextSlot)
    {
        TimerTextSlot->SetAutoSize(true);
        TimerTextSlot->SetPosition(FVector2D(40.0f, 134.0f));
    }

    if (RoundStateTextSlot)
    {
        RoundStateTextSlot->SetAutoSize(true);
        RoundStateTextSlot->SetPosition(FVector2D(40.0f, 158.0f));
    }

    if (ResultTextSlot)
    {
        ResultTextSlot->SetAutoSize(true);
        ResultTextSlot->SetPosition(FVector2D(40.0f, 182.0f));
    }

    if (NextRoundTextSlot)
    {
        NextRoundTextSlot->SetAutoSize(true);
        NextRoundTextSlot->SetPosition(FVector2D(40.0f, 206.0f));
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
        if (RoundScoreText)
        {
            RoundScoreText->SetText(FText::FromString(
                FString::Printf(TEXT("Round K / D: %d / %d"), PvPPlayerState->GetRoundKills(), PvPPlayerState->GetRoundDeaths())));
        }

        if (MatchScoreText)
        {
            MatchScoreText->SetText(FText::FromString(
                FString::Printf(TEXT("Match K / D: %d / %d"), PvPPlayerState->GetMatchKills(), PvPPlayerState->GetMatchDeaths())));
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

        if (ResultText)
        {
            if (PvPGameState->GetRoundState() == EPvPARoundState::RoundEnd)
            {
                ResultText->SetText(FText::FromString(FString::Printf(TEXT("Result: %s"), *GetRoundResultText(PlayerController, PvPGameState))));
            }
            else
            {
                ResultText->SetText(FText::GetEmpty());
            }
        }

        if (NextRoundText)
        {
            if (PvPGameState->GetRoundState() == EPvPARoundState::RoundEnd)
            {
                NextRoundText->SetText(FText::FromString(
                    FString::Printf(TEXT("Next Round In: %d"), PvPGameState->GetRemainingRoundEndTimeSeconds())));
            }
            else
            {
                NextRoundText->SetText(FText::GetEmpty());
            }
        }
    }
}

FString UPvPArenaHUDWidget::GetRoundResultText(const APlayerController* PlayerController, const APvPArenaGameState* GameState) const
{
    if (!PlayerController || !GameState)
    {
        return TEXT("Unknown");
    }

    const APvPArenaPlayerState* LocalPlayerState = PlayerController->GetPlayerState<APvPArenaPlayerState>();
    if (!LocalPlayerState)
    {
        return TEXT("Unknown");
    }

    const int32 LocalKills = LocalPlayerState->GetRoundKills();
    int32 HighestOpponentKills = TNumericLimits<int32>::Min();
    bool bFoundOpponent = false;

    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        const APvPArenaPlayerState* PvPState = Cast<APvPArenaPlayerState>(PlayerState);
        if (!PvPState || PvPState == LocalPlayerState)
        {
            continue;
        }

        HighestOpponentKills = FMath::Max(HighestOpponentKills, PvPState->GetRoundKills());
        bFoundOpponent = true;
    }

    if (!bFoundOpponent)
    {
        return TEXT("Pending");
    }

    if (LocalKills > HighestOpponentKills)
    {
        return TEXT("Win");
    }

    if (LocalKills < HighestOpponentKills)
    {
        return TEXT("Lose");
    }

    return TEXT("Draw");
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
