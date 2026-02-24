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
    FString RoundStateToText(uint8 RoundStateValue) const;
};
