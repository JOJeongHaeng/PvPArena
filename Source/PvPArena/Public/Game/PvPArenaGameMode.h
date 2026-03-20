#pragma once

#include "CoreMinimal.h"
#include "Game/PvPArenaGameState.h"
#include "GameFramework/GameModeBase.h"
#include "UObject/ObjectKey.h"
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

    static constexpr int32 IterationScoreLimitDefault = 3;
    static constexpr int32 IterationRoundWinsToWinDefault = 3;
    static constexpr int32 IterationRoundDurationSecondsDefault = 60;
    static constexpr int32 PlannedFinalScoreLimitDefault = 5;
    static constexpr int32 PlannedFinalRoundDurationSecondsDefault = 180;

    bool HasWinner() const { return bHasWinner; }
    int32 GetIterationScoreLimitDefault() const { return IterationScoreLimitDefault; }
    int32 GetIterationRoundWinsToWinDefault() const { return IterationRoundWinsToWinDefault; }
    int32 GetLobbyCountdownSeconds() const { return LobbyCountdownSeconds; }
    int32 GetIterationRoundDurationSecondsDefault() const { return IterationRoundDurationSecondsDefault; }
    int32 GetPlannedFinalScoreLimitDefault() const { return PlannedFinalScoreLimitDefault; }
    int32 GetPlannedFinalRoundDurationSecondsDefault() const { return PlannedFinalRoundDurationSecondsDefault; }
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
    bool ShouldEndMatchOnRoundWin(int32 RoundWins) const;

    UFUNCTION(BlueprintPure, Category = "Match")
    bool IsReadyToStartMatch(int32 ConnectedPlayers, int32 ReadyPlayers) const;

    UFUNCTION(BlueprintCallable, Category = "Match")
    void HandleLobbyReadyStateChanged(APvPArenaPlayerState* PlayerState, bool bReadyForStart);

    UFUNCTION(BlueprintPure, Category = "Match")
    bool ShouldScheduleRespawnAfterElimination(bool bHasVictimController) const;

    UFUNCTION(BlueprintCallable, Category = "Match")
    void HandlePlayerEliminated(AController* VictimController, AController* KillerController);

    AActor* ChooseRespawnStartFromCandidates(const TArray<AActor*>& CandidateStarts, const AActor* PreviousStart) const;
    AActor* ChooseRespawnStartForPlayer(const TArray<AActor*>& CandidateStarts, AController* Player);
    AActor* ChooseRoundStartFromCandidates(const TArray<AActor*>& CandidateStarts, AController* Player, const TSet<TObjectKey<AActor>>& UsedStarts) const;

protected:
    virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
    virtual bool ShouldSpawnAtStartSpot(AController* Player) override;

private:
    static constexpr int32 MinimumPlayersToStartMatch = 2;
    static constexpr int32 LobbyCountdownSeconds = 3;
    static constexpr int32 MatchEndDelaySeconds = 6;

    void EnterLobbyPhase(bool bResetMatchStats);
    void TryStartMatchFromLobby();
    void CancelLobbyCountdown();
    void OnLobbyCountdownSecondElapsed();
    void StartMatchFlow();
    void BeginMatchEndPhase(APvPArenaPlayerState* MatchWinner);
    void BeginRoundEndPhase();
    void AwardRoundWin(APvPArenaPlayerState* RoundWinner);
    APvPArenaPlayerState* ResolveRoundWinnerFromScores() const;
    int32 CountConnectedPlayers() const;
    int32 CountReadyPlayers() const;
    void ResetAllMatchStats();
    void OnMatchEndSecondElapsed();
    void OnRoundResetSecondElapsed();
    void HandleRoundReset();
    void ResetAllPlayersForNextRound();
    void StartRoundTimer();
    void OnRoundSecondElapsed();

    // Keep short iteration defaults active until the final completion pass restores 5 / 180.
    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 ScoreLimit = IterationScoreLimitDefault;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 RoundDurationSeconds = IterationRoundDurationSecondsDefault;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 RespawnDelaySeconds = 2;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 RoundEndDelaySeconds = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    float RespawnInvulnerabilitySeconds = 1.25f;

    bool bHasWinner = false;
    TMap<TObjectKey<AController>, TWeakObjectPtr<AActor>> LastChosenPlayerStartsByController;
    FTimerHandle RoundTimerHandle;
    FTimerHandle RoundResetTimerHandle;
    FTimerHandle LobbyCountdownTimerHandle;
    FTimerHandle MatchEndTimerHandle;
};
