#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PvPArenaHUD.generated.h"

UCLASS()
class PVPARENA_API APvPArenaHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;

private:
    FLinearColor GetRoundResultColor(const FString& RoundResultText) const;
    FString GetRoundResultText(class APlayerController* PC, const class APvPArenaGameState* GameState) const;
    FString RoundStateToText(uint8 RoundStateValue) const;
};
