#include "UI/PvPArenaHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "Game/PvPArenaGameState.h"
#include "Game/PvPArenaPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Player/PvPArenaCharacter.h"
#include "TimerManager.h"
#include "Widgets/SWidget.h"

TSharedRef<SWidget> UPvPArenaHUDWidget::RebuildWidget()
{
    BuildWidgetTree();
    return Super::RebuildWidget();
}

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

    if (RootOverlay)
    {
        return;
    }

    RootOverlay = Cast<UOverlay>(WidgetTree->RootWidget);
    if (!RootOverlay)
    {
        RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
        WidgetTree->RootWidget = RootOverlay;
    }

    StatusBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StatusBox"));
    AnnouncementBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("AnnouncementBox"));
    HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
    HealthText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HealthText"));
    RoundScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RoundScoreText"));
    MatchScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MatchScoreText"));
    TimerText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TimerText"));
    RoundStateText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RoundStateText"));
    ResultText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ResultText"));
    NextRoundText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NextRoundText"));

    if (!RootOverlay || !StatusBox || !AnnouncementBox || !HealthBar || !HealthText || !RoundScoreText || !MatchScoreText || !TimerText || !RoundStateText || !ResultText || !NextRoundText)
    {
        return;
    }

    UOverlaySlot* StatusBoxSlot = RootOverlay->AddChildToOverlay(StatusBox);
    UOverlaySlot* AnnouncementBoxSlot = RootOverlay->AddChildToOverlay(AnnouncementBox);

    if (StatusBoxSlot)
    {
        StatusBoxSlot->SetHorizontalAlignment(HAlign_Left);
        StatusBoxSlot->SetVerticalAlignment(VAlign_Top);
        StatusBoxSlot->SetPadding(FMargin(40.0f, 40.0f, 0.0f, 0.0f));
    }

    if (AnnouncementBoxSlot)
    {
        AnnouncementBoxSlot->SetHorizontalAlignment(HAlign_Center);
        AnnouncementBoxSlot->SetVerticalAlignment(VAlign_Center);
        AnnouncementBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 80.0f));
    }

    HealthBar->SetPercent(1.0f);
    HealthBar->SetFillColorAndOpacity(FLinearColor(0.9f, 0.2f, 0.2f, 1.0f));
    UVerticalBoxSlot* HealthBarSlot = StatusBox->AddChildToVerticalBox(HealthBar);
    UVerticalBoxSlot* HealthTextSlot = StatusBox->AddChildToVerticalBox(HealthText);
    UVerticalBoxSlot* RoundScoreTextSlot = StatusBox->AddChildToVerticalBox(RoundScoreText);
    UVerticalBoxSlot* MatchScoreTextSlot = StatusBox->AddChildToVerticalBox(MatchScoreText);
    UVerticalBoxSlot* TimerTextSlot = StatusBox->AddChildToVerticalBox(TimerText);
    UVerticalBoxSlot* RoundStateTextSlot = StatusBox->AddChildToVerticalBox(RoundStateText);
    UVerticalBoxSlot* ResultTextSlot = AnnouncementBox->AddChildToVerticalBox(ResultText);
    UVerticalBoxSlot* NextRoundTextSlot = AnnouncementBox->AddChildToVerticalBox(NextRoundText);

    if (HealthBarSlot)
    {
        HealthBarSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
    }

    if (HealthTextSlot)
    {
        HealthTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
    }
    HealthText->SetColorAndOpacity(FSlateColor(FLinearColor::White));

    if (RoundScoreTextSlot)
    {
        RoundScoreTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
    }
    RoundScoreText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));

    if (MatchScoreTextSlot)
    {
        MatchScoreTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
    }
    MatchScoreText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.85f, 0.2f, 1.0f)));

    if (TimerTextSlot)
    {
        TimerTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
    }
    TimerText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));

    if (RoundStateTextSlot)
    {
        RoundStateTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
    }
    RoundStateText->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 1.0f, 1.0f)));

    if (ResultTextSlot)
    {
        ResultTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }
    ResultText->SetJustification(ETextJustify::Center);
    ResultText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
    ResultText->SetFont(FSlateFontInfo(ResultText->GetFont().FontObject, 36, ResultText->GetFont().TypefaceFontName));
    ResultText->SetVisibility(ESlateVisibility::Collapsed);

    if (NextRoundTextSlot)
    {
        NextRoundTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
    }
    NextRoundText->SetJustification(ETextJustify::Center);
    NextRoundText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.6f, 0.0f, 1.0f)));
    NextRoundText->SetFont(FSlateFontInfo(NextRoundText->GetFont().FontObject, 24, NextRoundText->GetFont().TypefaceFontName));
    NextRoundText->SetVisibility(ESlateVisibility::Collapsed);
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
                ResultText->SetVisibility(ESlateVisibility::Visible);
                ResultText->SetText(FText::FromString(GetRoundResultText(PlayerController, PvPGameState)));
            }
            else
            {
                ResultText->SetVisibility(ESlateVisibility::Collapsed);
                ResultText->SetText(FText::GetEmpty());
            }
        }

        if (NextRoundText)
        {
            if (PvPGameState->GetRoundState() == EPvPARoundState::RoundEnd)
            {
                NextRoundText->SetVisibility(ESlateVisibility::Visible);
                NextRoundText->SetText(FText::FromString(
                    FString::Printf(TEXT("Next Round In: %d"), PvPGameState->GetRemainingRoundEndTimeSeconds())));
            }
            else
            {
                NextRoundText->SetVisibility(ESlateVisibility::Collapsed);
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
        return TEXT("Victory");
    }

    if (LocalKills < HighestOpponentKills)
    {
        return TEXT("Defeat");
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
