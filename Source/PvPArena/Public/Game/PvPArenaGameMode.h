#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PvPArenaGameMode.generated.h"

class APvPArenaPlayerState;

UCLASS()
class PVPARENA_API APvPArenaGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    APvPArenaGameMode();

    bool HasWinner() const { return bHasWinner; }

    UFUNCTION(BlueprintCallable, Category = "Match")
    void RegisterKill(APvPArenaPlayerState* Killer, APvPArenaPlayerState* Victim);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 ScoreLimit = 5;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 RoundDurationSeconds = 180;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 RespawnDelaySeconds = 3;

    bool bHasWinner = false;
};
