#pragma once

#include "CoreMinimal.h"
#include "Game/PvPArenaGameState.h"
#include "GameFramework/GameModeBase.h"
#include "PvPArenaGameMode.generated.h"

class APvPArenaPlayerState;

UCLASS()
class PVPARENA_API APvPArenaGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    APvPArenaGameMode();

    virtual void PostLogin(APlayerController* NewPlayer) override;

    bool TryAssignTeam(APvPArenaPlayerState* InPlayerState, int32 DesiredTeamId);
    UFUNCTION(BlueprintCallable, Category = "Match")
    bool EvaluateEndConditions();

protected:
    virtual void BeginPlay() override;

private:
    bool IsLobbyPhase() const;
    int32 CountPlayersOnTeam(int32 TeamId) const;
    void StartPhase(EPvPArenaMatchPhase NewPhase, int32 DurationSeconds);
    void HandlePhaseTick();

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 LobbyDurationSeconds;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 CombatDurationSeconds;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 ResultDurationSeconds;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 KillLimit;

    FTimerHandle PhaseTimerHandle;
};
