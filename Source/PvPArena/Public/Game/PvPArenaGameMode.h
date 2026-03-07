#pragma once

#include "CoreMinimal.h"
#include "Game/PvPArenaGameState.h"
#include "GameFramework/GameModeBase.h"
#include "PvPArenaGameMode.generated.h"

class APvPArenaPlayerState;
class APvPArenaCharacter;
class AController;

UCLASS()
class PVPARENA_API APvPArenaGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    APvPArenaGameMode();

    bool HasWinner() const { return bHasWinner; }

    UFUNCTION(BlueprintCallable, Category = "Match")
    void RegisterKill(APvPArenaPlayerState* Killer, APvPArenaPlayerState* Victim);

    UFUNCTION(BlueprintCallable, Category = "Match")
    EPvPARoundState ResolveRoundTimeout(int32 PlayerOneScore, int32 PlayerTwoScore);

    UFUNCTION(BlueprintPure, Category = "Match")
    bool ShouldEndRoundOnKill(EPvPARoundState CurrentRoundState, int32 KillerKills) const;

    UFUNCTION(BlueprintCallable, Category = "Match")
    void HandlePlayerEliminated(AController* VictimController, AController* KillerController);

protected:
    virtual void BeginPlay() override;

private:
    void BeginRoundEndPhase();
    void OnRoundResetSecondElapsed();
    void HandleRoundReset();
    void ResetAllPlayersForNextRound();
    void StartRoundTimer();
    void OnRoundSecondElapsed();

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 ScoreLimit = 5;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 RoundDurationSeconds = 180;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 RespawnDelaySeconds = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 RoundEndDelaySeconds = 5;

    bool bHasWinner = false;
    FTimerHandle RoundTimerHandle;
    FTimerHandle RoundResetTimerHandle;
};
