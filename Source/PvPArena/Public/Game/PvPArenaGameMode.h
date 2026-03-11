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
    int32 GetRespawnDelaySeconds() const { return RespawnDelaySeconds; }
    int32 GetRoundEndDelaySeconds() const { return RoundEndDelaySeconds; }
    float GetRespawnInvulnerabilitySeconds() const { return RespawnInvulnerabilitySeconds; }

    UFUNCTION(BlueprintCallable, Category = "Match")
    void RegisterKill(APvPArenaPlayerState* Killer, APvPArenaPlayerState* Victim);

    UFUNCTION(BlueprintCallable, Category = "Match")
    EPvPARoundState ResolveRoundTimeout(int32 PlayerOneScore, int32 PlayerTwoScore);

    UFUNCTION(BlueprintPure, Category = "Match")
    bool ShouldEndRoundOnKill(EPvPARoundState CurrentRoundState, int32 KillerKills) const;

    UFUNCTION(BlueprintPure, Category = "Match")
    bool ShouldScheduleRespawnAfterElimination(bool bHasVictimController) const;

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
    int32 ScoreLimit = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 RoundDurationSeconds = 60;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 RespawnDelaySeconds = 2;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 RoundEndDelaySeconds = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    float RespawnInvulnerabilitySeconds = 1.25f;

    bool bHasWinner = false;
    FTimerHandle RoundTimerHandle;
    FTimerHandle RoundResetTimerHandle;
};
