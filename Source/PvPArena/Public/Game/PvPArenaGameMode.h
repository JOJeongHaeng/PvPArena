#pragma once

#include "CoreMinimal.h"
#include "Game/PvPArenaGameState.h"
#include "Game/PvPArenaPlayerState.h"
#include "GameFramework/GameModeBase.h"
#include "UObject/ObjectKey.h"
#include "PvPArenaGameMode.generated.h"

class APvPArenaCharacter;
class AController;
class ASpectatorPawn;

UCLASS()
class PVPARENA_API APvPArenaGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    APvPArenaGameMode();

    static constexpr int32 IterationScoreLimitDefault = 5;
    static constexpr int32 IterationRoundWinsToWinDefault = 3;
    static constexpr int32 IterationRoundDurationSecondsDefault = 180;
    static constexpr int32 TeamVersusRoundDurationSecondsDefault = 60;
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
    TSubclassOf<ASpectatorPawn> GetConfiguredSpectatorClass() const { return SpectatorClass; }

    static FString BuildDefaultDisplayNickname(int32 PlayerIndex);

    UFUNCTION(BlueprintCallable, Category = "Match")
    void RegisterKill(APvPArenaPlayerState* Killer, APvPArenaPlayerState* Victim);

    UFUNCTION(BlueprintCallable, Category = "Match")
    EPvPARoundState ResolveRoundTimeout(int32 PlayerOneScore, int32 PlayerTwoScore);

    UFUNCTION(BlueprintPure, Category = "Match")
    EPvPARoundState ResolveFreeForAllRoundTimeoutState(bool bHasTieForLead) const;

    UFUNCTION(BlueprintPure, Category = "Match")
    EPvPARoundState ResolveTeamVersusRoundTimeoutState(int32 LeftTeamAlivePlayers, int32 RightTeamAlivePlayers) const;

    UFUNCTION(BlueprintPure, Category = "Match")
    bool ShouldEndRoundOnKill(EPvPARoundState CurrentRoundState, int32 KillerKills) const;

    UFUNCTION(BlueprintPure, Category = "Match")
    bool ShouldEndMatchOnRoundWin(int32 RoundWins) const;

    UFUNCTION(BlueprintPure, Category = "Match")
    bool ShouldEndMatchOnRoundWinState(int32 HighestRoundWins, bool bHasTieAtHighestRoundWins) const;

    UFUNCTION(BlueprintPure, Category = "Match")
    EPvPAMatchPhase GetInitialMatchPhase() const { return EPvPAMatchPhase::Lobby; }

    UFUNCTION(BlueprintPure, Category = "Match")
    EPvPAMatchPhase ResolveMatchPhaseAfterRoundWin(int32 RoundWins) const;

    UFUNCTION(BlueprintPure, Category = "Match")
    EPvPAMatchPhase ResolveMatchPhaseAfterRoundWinState(int32 HighestRoundWins, bool bHasTieAtHighestRoundWins) const;

    UFUNCTION(BlueprintPure, Category = "Match")
    bool ShouldContinueToNextRoundAfterRoundWin(int32 RoundWins) const;

    UFUNCTION(BlueprintPure, Category = "Match")
    bool IsReadyToStartMatch(EPvPALobbyMatchMode LobbyMatchMode, int32 ConnectedPlayers, int32 LeftTeamPlayers, int32 RightTeamPlayers) const;

    UFUNCTION(BlueprintPure, Category = "Match")
    bool CanLobbyHostStartMatch(bool bRequestingControllerHasAuthority, int32 ConnectedPlayers) const;

    UFUNCTION(BlueprintPure, Category = "Match")
    int32 GetMaximumLobbyPlayers() const { return MaximumLobbyPlayers; }

    UFUNCTION(BlueprintPure, Category = "Match")
    bool ShouldReturnToLobbyAfterMatchEnd(int32 RemainingMatchEndSeconds) const;

    UFUNCTION(BlueprintCallable, Category = "Match")
    void RequestLobbyMatchStart(AController* RequestingController);

    UFUNCTION(BlueprintCallable, Category = "Match")
    void HandleLobbyReadyStateChanged(APvPArenaPlayerState* PlayerState, bool bReadyForStart);

    UFUNCTION(BlueprintCallable, Category = "Match")
    void HandleLobbyDisplayNicknameChanged(APvPArenaPlayerState* PlayerState, const FString& RequestedNickname);
    void HandleLobbyMatchModeChanged(AController* RequestingController, EPvPALobbyMatchMode NewLobbyMatchMode);

    UFUNCTION(BlueprintCallable, Category = "Match")
    void HandleLobbyTeamSelectionChanged(APvPArenaPlayerState* PlayerState, EPvPALobbyTeam NewLobbyTeam);

    UFUNCTION(BlueprintCallable, Category = "Match")
    void ApplyLobbyModeChangeToPlayerState(APvPArenaPlayerState* PlayerState, EPvPALobbyMatchMode NewLobbyMatchMode) const;

    UFUNCTION(BlueprintPure, Category = "Match")
    bool ShouldScheduleRespawnAfterElimination(bool bHasVictimController) const;

    UFUNCTION(BlueprintPure, Category = "Match")
    bool ShouldScheduleRespawnAfterEliminationForMode(
        EPvPALobbyMatchMode LobbyMatchMode,
        bool bHasVictimController,
        bool bRoundAlreadyHasWinner,
        EPvPARoundState CurrentRoundState = EPvPARoundState::Playing) const;

    UFUNCTION(BlueprintPure, Category = "Match")
    bool ShouldEnterSpectatingAfterEliminationForMode(
        EPvPALobbyMatchMode LobbyMatchMode,
        bool bHasVictimController,
        bool bRoundAlreadyHasWinner,
        EPvPARoundState CurrentRoundState = EPvPARoundState::Playing) const;

    UFUNCTION(BlueprintPure, Category = "Match")
    bool ShouldFinalizeDelayedSpectating(const APawn* CurrentPawn, const APawn* EliminatedPawn) const;

    UFUNCTION(BlueprintCallable, Category = "Match")
    void HandlePlayerEliminated(AController* VictimController, AController* KillerController);

    AActor* ChooseRespawnStartFromCandidates(const TArray<AActor*>& CandidateStarts, const AActor* PreviousStart) const;
    AActor* ChooseRespawnStartForPlayer(const TArray<AActor*>& CandidateStarts, AController* Player);
    AActor* ChooseRoundStartFromCandidates(const TArray<AActor*>& CandidateStarts, AController* Player, const TSet<TObjectKey<AActor>>& UsedStarts) const;
    TArray<AActor*> FilterPlayerStartsForPlayer(const TArray<AActor*>& CandidateStarts, const AController* Player) const;

protected:
    virtual void BeginPlay() override;
    virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
    virtual bool ShouldSpawnAtStartSpot(AController* Player) override;

private:
    static constexpr int32 MinimumPlayersToStartMatch = 2;
    static constexpr int32 MaximumLobbyPlayers = 6;
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
    void AwardRoundWinners(const TArray<APvPArenaPlayerState*>& RoundWinners, APvPArenaPlayerState* DisplayWinner);
    APvPArenaPlayerState* ResolveRoundWinnerFromScores() const;
    TArray<APvPArenaPlayerState*> ResolveRoundLeadersFromScores() const;
    TArray<APvPArenaPlayerState*> GatherPlayersOnLobbyTeam(EPvPALobbyTeam LobbyTeam) const;
    int32 CountAlivePlayersOnLobbyTeam(EPvPALobbyTeam LobbyTeam) const;
    int32 GetTeamRoundWins(EPvPALobbyTeam LobbyTeam) const;
    APvPArenaPlayerState* ResolveRepresentingPlayerForTeam(EPvPALobbyTeam LobbyTeam) const;
    APvPArenaPlayerState* ResolveUniqueMatchWinner() const;
    bool HasTieAtHighestRoundWins(int32& OutHighestRoundWins) const;
    int32 ResolveScoreLimitForMode(EPvPALobbyMatchMode LobbyMatchMode) const;
    int32 ResolveRoundDurationSecondsForMode(EPvPALobbyMatchMode LobbyMatchMode) const;
    int32 ResolveRoundWinsToWinForMode(EPvPALobbyMatchMode LobbyMatchMode) const;
    void EnterFreeForAllSuddenDeath(const TArray<APvPArenaPlayerState*>& TiedLeaders);
    void ResetControllersForFreeForAllSuddenDeath(const TArray<APvPArenaPlayerState*>& TiedLeaders);
    void SetControllerSpectating(AController* Controller) const;
    EPvPALobbyMatchMode GetLobbyMatchMode() const;
    int32 CountConnectedPlayers() const;
    int32 CountReadyPlayers() const;
    FString ResolveUniqueDefaultDisplayNickname(const APvPArenaPlayerState* ExcludedPlayerState = nullptr) const;
    int32 CountPlayersOnLobbyTeam(EPvPALobbyTeam LobbyTeam) const;
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
