#include "UI/PvPArenaHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "Game/PvPArenaGameState.h"
#include "Game/PvPArenaPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Combat/PvPCombatComponent.h"
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
    RefreshCrosshairVisibility();
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

void UPvPArenaHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    RefreshCrosshairVisibility();
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

    StatusPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("StatusPanel"));
    AnnouncementPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("AnnouncementPanel"));
    StatusBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StatusBox"));
    AnnouncementBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("AnnouncementBox"));
    HealthBarSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("HealthBarSizeBox"));
    HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
    RangedCooldownBarSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RangedCooldownBarSizeBox"));
    RangedCooldownBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("RangedCooldownBar"));
    HealthText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HealthText"));
    RangedCooldownText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RangedCooldownText"));
    RoundScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RoundScoreText"));
    MatchScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MatchScoreText"));
    TimerText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TimerText"));
    RoundStateText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RoundStateText"));
    ResultText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ResultText"));
    NextRoundText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NextRoundText"));
    RangedCrosshairOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RangedCrosshairOverlay"));
    RangedCrosshairHorizontalBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RangedCrosshairHorizontalBox"));
    RangedCrosshairVerticalBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RangedCrosshairVerticalBox"));
    RangedCrosshairHorizontalLine = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RangedCrosshairHorizontalLine"));
    RangedCrosshairVerticalLine = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RangedCrosshairVerticalLine"));

    if (!RootOverlay || !StatusPanel || !AnnouncementPanel || !StatusBox || !AnnouncementBox || !HealthBarSizeBox || !HealthBar || !RangedCooldownBarSizeBox || !RangedCooldownBar || !HealthText || !RangedCooldownText || !RoundScoreText || !MatchScoreText || !TimerText || !RoundStateText || !ResultText || !NextRoundText || !RangedCrosshairOverlay || !RangedCrosshairHorizontalBox || !RangedCrosshairVerticalBox || !RangedCrosshairHorizontalLine || !RangedCrosshairVerticalLine)
    {
        return;
    }

    StatusPanel->SetContent(StatusBox);
    StatusPanel->SetPadding(FMargin(18.0f, 16.0f, 18.0f, 16.0f));
    StatusPanel->SetBrushColor(FLinearColor(0.03f, 0.05f, 0.08f, 0.78f));

    AnnouncementPanel->SetContent(AnnouncementBox);
    AnnouncementPanel->SetPadding(FMargin(28.0f, 20.0f, 28.0f, 20.0f));
    AnnouncementPanel->SetBrushColor(FLinearColor(0.02f, 0.03f, 0.06f, 0.82f));
    AnnouncementPanel->SetVisibility(ESlateVisibility::Collapsed);

    UOverlaySlot* StatusBoxSlot = RootOverlay->AddChildToOverlay(StatusPanel);
    UOverlaySlot* AnnouncementBoxSlot = RootOverlay->AddChildToOverlay(AnnouncementPanel);
    UOverlaySlot* RangedCrosshairSlot = RootOverlay->AddChildToOverlay(RangedCrosshairOverlay);

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

    if (RangedCrosshairSlot)
    {
        RangedCrosshairSlot->SetHorizontalAlignment(HAlign_Center);
        RangedCrosshairSlot->SetVerticalAlignment(VAlign_Center);
    }

    RangedCrosshairHorizontalBox->SetWidthOverride(18.0f);
    RangedCrosshairHorizontalBox->SetHeightOverride(2.0f);
    RangedCrosshairHorizontalBox->SetContent(RangedCrosshairHorizontalLine);
    RangedCrosshairVerticalBox->SetWidthOverride(2.0f);
    RangedCrosshairVerticalBox->SetHeightOverride(18.0f);
    RangedCrosshairVerticalBox->SetContent(RangedCrosshairVerticalLine);

    UOverlaySlot* HorizontalCrosshairSlot = RangedCrosshairOverlay->AddChildToOverlay(RangedCrosshairHorizontalBox);
    UOverlaySlot* VerticalCrosshairSlot = RangedCrosshairOverlay->AddChildToOverlay(RangedCrosshairVerticalBox);
    if (HorizontalCrosshairSlot)
    {
        HorizontalCrosshairSlot->SetHorizontalAlignment(HAlign_Center);
        HorizontalCrosshairSlot->SetVerticalAlignment(VAlign_Center);
        HorizontalCrosshairSlot->SetPadding(FMargin(-9.0f, 0.0f, -9.0f, 0.0f));
    }

    if (VerticalCrosshairSlot)
    {
        VerticalCrosshairSlot->SetHorizontalAlignment(HAlign_Center);
        VerticalCrosshairSlot->SetVerticalAlignment(VAlign_Center);
        VerticalCrosshairSlot->SetPadding(FMargin(0.0f, -9.0f, 0.0f, -9.0f));
    }

    HealthBarSizeBox->SetWidthOverride(280.0f);
    HealthBarSizeBox->SetHeightOverride(22.0f);
    HealthBarSizeBox->SetContent(HealthBar);

    RangedCooldownBarSizeBox->SetWidthOverride(280.0f);
    RangedCooldownBarSizeBox->SetHeightOverride(16.0f);
    RangedCooldownBarSizeBox->SetContent(RangedCooldownBar);

    HealthBar->SetPercent(1.0f);
    HealthBar->SetFillColorAndOpacity(FLinearColor(0.9f, 0.2f, 0.2f, 1.0f));
    RangedCooldownBar->SetPercent(1.0f);
    RangedCooldownBar->SetFillColorAndOpacity(FLinearColor(0.2f, 0.75f, 1.0f, 1.0f));
    UVerticalBoxSlot* HealthBarSlot = StatusBox->AddChildToVerticalBox(HealthBarSizeBox);
    UVerticalBoxSlot* HealthTextSlot = StatusBox->AddChildToVerticalBox(HealthText);
    UVerticalBoxSlot* RangedCooldownBarSlot = StatusBox->AddChildToVerticalBox(RangedCooldownBarSizeBox);
    UVerticalBoxSlot* RangedCooldownTextSlot = StatusBox->AddChildToVerticalBox(RangedCooldownText);
    UVerticalBoxSlot* RoundScoreTextSlot = StatusBox->AddChildToVerticalBox(RoundScoreText);
    UVerticalBoxSlot* MatchScoreTextSlot = StatusBox->AddChildToVerticalBox(MatchScoreText);
    UVerticalBoxSlot* TimerTextSlot = StatusBox->AddChildToVerticalBox(TimerText);
    UVerticalBoxSlot* RoundStateTextSlot = StatusBox->AddChildToVerticalBox(RoundStateText);
    UVerticalBoxSlot* ResultTextSlot = AnnouncementBox->AddChildToVerticalBox(ResultText);
    UVerticalBoxSlot* NextRoundTextSlot = AnnouncementBox->AddChildToVerticalBox(NextRoundText);

    if (HealthBarSlot)
    {
        HealthBarSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    }

    if (HealthTextSlot)
    {
        HealthTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }
    HealthText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    HealthText->SetFont(FSlateFontInfo(HealthText->GetFont().FontObject, 20, HealthText->GetFont().TypefaceFontName));
    HealthText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (RangedCooldownBarSlot)
    {
        RangedCooldownBarSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }

    if (RangedCooldownTextSlot)
    {
        RangedCooldownTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    }
    RangedCooldownText->SetColorAndOpacity(FSlateColor(FLinearColor(0.55f, 0.9f, 1.0f, 1.0f)));
    RangedCooldownText->SetFont(FSlateFontInfo(RangedCooldownText->GetFont().FontObject, 18, RangedCooldownText->GetFont().TypefaceFontName));
    RangedCooldownText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (RoundScoreTextSlot)
    {
        RoundScoreTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
    }
    RoundScoreText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
    RoundScoreText->SetFont(FSlateFontInfo(RoundScoreText->GetFont().FontObject, 18, RoundScoreText->GetFont().TypefaceFontName));
    RoundScoreText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (MatchScoreTextSlot)
    {
        MatchScoreTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
    }
    MatchScoreText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.85f, 0.2f, 1.0f)));
    MatchScoreText->SetFont(FSlateFontInfo(MatchScoreText->GetFont().FontObject, 18, MatchScoreText->GetFont().TypefaceFontName));
    MatchScoreText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (TimerTextSlot)
    {
        TimerTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
    }
    TimerText->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 1.0f, 0.55f, 1.0f)));
    TimerText->SetFont(FSlateFontInfo(TimerText->GetFont().FontObject, 18, TimerText->GetFont().TypefaceFontName));
    TimerText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (RoundStateTextSlot)
    {
        RoundStateTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
    }
    RoundStateText->SetColorAndOpacity(FSlateColor(FLinearColor(0.45f, 0.9f, 1.0f, 1.0f)));
    RoundStateText->SetFont(FSlateFontInfo(RoundStateText->GetFont().FontObject, 18, RoundStateText->GetFont().TypefaceFontName));
    RoundStateText->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (ResultTextSlot)
    {
        ResultTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    }
    ResultText->SetJustification(ETextJustify::Center);
    ResultText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
    ResultText->SetFont(FSlateFontInfo(ResultText->GetFont().FontObject, 42, ResultText->GetFont().TypefaceFontName));
    ResultText->SetShadowOffset(FVector2D(2.0f, 2.0f));
    ResultText->SetVisibility(ESlateVisibility::Collapsed);

    if (NextRoundTextSlot)
    {
        NextRoundTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
    }
    NextRoundText->SetJustification(ETextJustify::Center);
    NextRoundText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.6f, 0.0f, 1.0f)));
    NextRoundText->SetFont(FSlateFontInfo(NextRoundText->GetFont().FontObject, 28, NextRoundText->GetFont().TypefaceFontName));
    NextRoundText->SetShadowOffset(FVector2D(1.0f, 1.0f));
    NextRoundText->SetVisibility(ESlateVisibility::Collapsed);

    RangedCrosshairHorizontalLine->SetBrushColor(FLinearColor(0.95f, 0.98f, 1.0f, 0.95f));
    RangedCrosshairVerticalLine->SetBrushColor(FLinearColor(0.95f, 0.98f, 1.0f, 0.95f));
    RangedCrosshairOverlay->SetVisibility(ESlateVisibility::Collapsed);
}

void UPvPArenaHUDWidget::BuildHealthDisplayState(const APvPArenaCharacter* Character, float& OutHealthPercent, FString& OutHealthLabel)
{
    if (!Character)
    {
        OutHealthPercent = 0.0f;
        OutHealthLabel = TEXT("HP: --");
        return;
    }

    const float MaxHp = FMath::Max(1.0f, Character->GetMaxHealth());
    const float CurrentHp = Character->GetCurrentHealth();
    OutHealthPercent = CurrentHp / MaxHp;
    OutHealthLabel = FString::Printf(TEXT("HP: %.0f / %.0f"), CurrentHp, MaxHp);
}

void UPvPArenaHUDWidget::BuildRangedCooldownDisplayState(const UPvPCombatComponent* CombatComponent, float NowSeconds, float& OutCooldownPercent, FString& OutCooldownLabel)
{
    if (!CombatComponent)
    {
        OutCooldownPercent = 0.0f;
        OutCooldownLabel = TEXT("Ranged: --");
        return;
    }

    const float RemainingCooldown = CombatComponent->GetRemainingRangedCooldown(NowSeconds);
    OutCooldownPercent = CombatComponent->GetRangedCooldownAlpha(NowSeconds);
    OutCooldownLabel = RemainingCooldown > 0.0f
        ? FString::Printf(TEXT("Ranged: %.1fs"), RemainingCooldown)
        : FString(TEXT("Ranged: Ready"));
}

ESlateVisibility UPvPArenaHUDWidget::BuildRangedCrosshairVisibilityState(const APvPArenaCharacter* Character)
{
    return Character && Character->IsRangedChargeInputHeld()
        ? ESlateVisibility::Visible
        : ESlateVisibility::Collapsed;
}

void UPvPArenaHUDWidget::RefreshWidgetData()
{
    APlayerController* PlayerController = GetOwningPlayer();
    if (!PlayerController)
    {
        return;
    }

    float HealthPercent = 0.0f;
    FString HealthLabel;
    const APvPArenaCharacter* Character = Cast<APvPArenaCharacter>(PlayerController->GetPawn());
    BuildHealthDisplayState(Character, HealthPercent, HealthLabel);

    float RangedCooldownPercent = 0.0f;
    FString RangedCooldownLabel;
    const APvPArenaGameState* PvPGameState = GetWorld() ? GetWorld()->GetGameState<APvPArenaGameState>() : nullptr;
    const float CooldownNowSeconds = PvPGameState ? PvPGameState->GetServerWorldTimeSeconds() : (GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f);
    BuildRangedCooldownDisplayState(
        Character ? Character->GetCombatComponent() : nullptr,
        CooldownNowSeconds,
        RangedCooldownPercent,
        RangedCooldownLabel);

    RefreshCrosshairVisibility();

    if (HealthBar)
    {
        HealthBar->SetPercent(HealthPercent);
    }

    if (HealthText)
    {
        HealthText->SetText(FText::FromString(HealthLabel));
    }

    if (RangedCooldownBar)
    {
        RangedCooldownBar->SetPercent(RangedCooldownPercent);
    }

    if (RangedCooldownText)
    {
        RangedCooldownText->SetText(FText::FromString(RangedCooldownLabel));
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

    if (PvPGameState)
    {
        const bool bIsRoundEnd = PvPGameState->GetRoundState() == EPvPARoundState::RoundEnd;

        if (AnnouncementPanel)
        {
            AnnouncementPanel->SetVisibility(bIsRoundEnd ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }

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
            if (bIsRoundEnd)
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
            if (bIsRoundEnd)
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

void UPvPArenaHUDWidget::RefreshCrosshairVisibility()
{
    if (!RangedCrosshairOverlay)
    {
        return;
    }

    const APlayerController* PlayerController = GetOwningPlayer();
    const APvPArenaCharacter* Character = PlayerController ? Cast<APvPArenaCharacter>(PlayerController->GetPawn()) : nullptr;
    RangedCrosshairOverlay->SetVisibility(BuildRangedCrosshairVisibilityState(Character));
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
