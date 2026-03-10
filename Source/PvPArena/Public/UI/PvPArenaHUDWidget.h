#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PvPArenaHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UOverlay;
class UVerticalBox;

UCLASS()
class PVPARENA_API UPvPArenaHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    static void BuildHealthDisplayState(const class APvPArenaCharacter* Character, float& OutHealthPercent, FString& OutHealthLabel);

private:
    void BuildWidgetTree();
    void RefreshWidgetData();
    FString GetRoundResultText(const class APlayerController* PlayerController, const class APvPArenaGameState* GameState) const;
    static FString RoundStateToString(uint8 RoundStateValue);

    UPROPERTY(Transient)
    TObjectPtr<UOverlay> RootOverlay;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> StatusBox;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> AnnouncementBox;

    UPROPERTY(Transient)
    TObjectPtr<UProgressBar> HealthBar;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> HealthText;

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

    FTimerHandle RefreshTimerHandle;
};
