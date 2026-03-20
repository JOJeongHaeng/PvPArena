#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PvPArenaHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UButton;
class UOverlay;
class UBorder;
class USizeBox;
class UHorizontalBox;
class UVerticalBox;
enum class ESlateVisibility : uint8;
struct FGeometry;
class UPvPCombatComponent;

UCLASS()
class PVPARENA_API UPvPArenaHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    static void BuildHealthDisplayState(const class APvPArenaCharacter* Character, float& OutHealthPercent, FString& OutHealthLabel);
    static void BuildSprintDisplayState(const class APvPArenaCharacter* Character, float& OutSprintPercent, FString& OutSprintLabel);
    static void BuildRangedCooldownDisplayState(const UPvPCombatComponent* CombatComponent, float NowSeconds, float& OutCooldownPercent, FString& OutCooldownLabel);
    static ESlateVisibility BuildRangedCrosshairVisibilityState(const class APvPArenaCharacter* Character);

private:
    UFUNCTION()
    void HandleLobbyReadyButtonClicked();

    void BuildWidgetTree();
    void RefreshWidgetData();
    void RefreshCrosshairVisibility();
    void ApplyLobbyInputMode(class APlayerController* PlayerController, bool bEnableLobbyInput);
    FString GetLobbyStatusText(const class APvPArenaGameState* GameState) const;
    FString GetRoundResultText(const class APlayerController* PlayerController, const class APvPArenaGameState* GameState) const;
    FString GetMatchResultText(const class APlayerController* PlayerController, const class APvPArenaGameState* GameState) const;
    FString GetMatchSummaryText(const class APlayerController* PlayerController, const class APvPArenaGameState* GameState) const;
    static FString MatchPhaseToString(uint8 MatchPhaseValue);
    static FString RoundStateToString(uint8 RoundStateValue);

    UPROPERTY(Transient)
    TObjectPtr<UOverlay> RootOverlay;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> StatusPanel;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> InfoPanel;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> AnnouncementPanel;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> LobbyPanel;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> LobbyControlsPanel;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> MatchResultPanel;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> StatusBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> InfoBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> AnnouncementBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> LobbyBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> LobbyControlsBox;

    UPROPERTY(Transient)
    TObjectPtr<UHorizontalBox> LobbyControlsCardsBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> MatchResultBox;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> LobbyKeyboardCard;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> LobbyMouseCard;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> LobbyKeyboardCardBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> LobbyMouseCardBox;

    UPROPERTY(Transient)
    TObjectPtr<USizeBox> HealthBarSizeBox;

    UPROPERTY(Transient)
    TObjectPtr<UProgressBar> HealthBar;

    UPROPERTY(Transient)
    TObjectPtr<USizeBox> SprintBarSizeBox;

    UPROPERTY(Transient)
    TObjectPtr<UProgressBar> SprintBar;

    UPROPERTY(Transient)
    TObjectPtr<USizeBox> RangedCooldownBarSizeBox;

    UPROPERTY(Transient)
    TObjectPtr<UProgressBar> RangedCooldownBar;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> HealthText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SprintText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> RangedCooldownText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> RoundScoreText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> MatchScoreText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> TimerText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> RoundStateText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> ResultText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> NextRoundText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyTitleText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyStatusText;

    UPROPERTY(Transient)
    TObjectPtr<UButton> LobbyReadyButton;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyReadyButtonText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyControlsTitleText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyControlsKeyboardTitleText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyControlsKeyboardMoveText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyControlsKeyboardSprintText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyControlsMouseTitleText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyControlsMouseMeleeText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LobbyControlsMouseRangedText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> MatchResultTitleText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> MatchResultSummaryText;

    UPROPERTY(Transient)
    TObjectPtr<UOverlay> RangedCrosshairOverlay;

    UPROPERTY(Transient)
    TObjectPtr<USizeBox> RangedCrosshairHorizontalBox;

    UPROPERTY(Transient)
    TObjectPtr<USizeBox> RangedCrosshairVerticalBox;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> RangedCrosshairHorizontalLine;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> RangedCrosshairVerticalLine;

    FTimerHandle RefreshTimerHandle;
    bool bLobbyInputModeActive = false;
};
