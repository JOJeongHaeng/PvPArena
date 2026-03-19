#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PvPArenaHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UOverlay;
class UBorder;
class USizeBox;
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
    static void BuildRangedCooldownDisplayState(const UPvPCombatComponent* CombatComponent, float NowSeconds, float& OutCooldownPercent, FString& OutCooldownLabel);
    static ESlateVisibility BuildRangedCrosshairVisibilityState(const class APvPArenaCharacter* Character);

private:
    void BuildWidgetTree();
    void RefreshWidgetData();
    void RefreshCrosshairVisibility();
    FString GetRoundResultText(const class APlayerController* PlayerController, const class APvPArenaGameState* GameState) const;
    static FString RoundStateToString(uint8 RoundStateValue);

    UPROPERTY(Transient)
    TObjectPtr<UOverlay> RootOverlay;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> StatusPanel;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> AnnouncementPanel;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> StatusBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> AnnouncementBox;

    UPROPERTY(Transient)
    TObjectPtr<USizeBox> HealthBarSizeBox;

    UPROPERTY(Transient)
    TObjectPtr<UProgressBar> HealthBar;

    UPROPERTY(Transient)
    TObjectPtr<USizeBox> RangedCooldownBarSizeBox;

    UPROPERTY(Transient)
    TObjectPtr<UProgressBar> RangedCooldownBar;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> HealthText;

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
    TObjectPtr<UTextBlock> RangedCrosshairText;

    FTimerHandle RefreshTimerHandle;
};
