#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Game/PvPArenaGameState.h"
#include "PvPArenaGameMode.generated.h"

UCLASS()
class PVPARENA_API APvPArenaGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    APvPArenaGameMode();

protected:
    virtual void BeginPlay() override;

private:
    void StartPhase(EPvPArenaMatchPhase NewPhase, int32 DurationSeconds);
    void HandlePhaseTick();

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 LobbyDurationSeconds;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 CombatDurationSeconds;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 ResultDurationSeconds;

    FTimerHandle PhaseTimerHandle;
};
