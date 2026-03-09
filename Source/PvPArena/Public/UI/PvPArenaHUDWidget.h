#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PvPArenaHUDWidget.generated.h"

class UCanvasPanel;
class UProgressBar;
class UTextBlock;

UCLASS()
class PVPARENA_API UPvPArenaHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    void BuildWidgetTree();
    void RefreshWidgetData();
    FString GetRoundResultText(const class APlayerController* PlayerController, const class APvPArenaGameState* GameState) const;
    static FString RoundStateToString(uint8 RoundStateValue);

    UPROPERTY(Transient)
    TObjectPtr<UCanvasPanel> RootCanvas;

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
