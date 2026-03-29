#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PvPArenaGameState.generated.h"

UENUM(BlueprintType)
enum class EPvPARoundState : uint8
{
    Playing,
    RoundEnd,
    SuddenDeath
};

UENUM(BlueprintType)
enum class EPvPAMatchPhase : uint8
{
    Lobby,
    Playing,
    MatchEnd
};

class APvPArenaPlayerState;

UCLASS()
class PVPARENA_API APvPArenaGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    int32 GetRemainingRoundTimeSeconds() const { return RemainingRoundTimeSeconds; }
    int32 GetRemainingRoundEndTimeSeconds() const { return RemainingRoundEndTimeSeconds; }
    int32 GetRemainingLobbyCountdownSeconds() const { return RemainingLobbyCountdownSeconds; }
    int32 GetRemainingMatchEndTimeSeconds() const { return RemainingMatchEndTimeSeconds; }
    int32 GetScoreLimit() const { return ScoreLimit; }
    int32 GetRoundWinsToWin() const { return RoundWinsToWin; }
    EPvPARoundState GetRoundState() const { return RoundState; }
    EPvPAMatchPhase GetMatchPhase() const { return MatchPhase; }
    const APvPArenaPlayerState* GetRoundWinner() const { return RoundWinner; }
    const APvPArenaPlayerState* GetMatchWinner() const { return MatchWinner; }

    void SetRemainingRoundTimeSeconds(int32 NewTime);
    void SetRemainingRoundEndTimeSeconds(int32 NewTime);
    void SetRemainingLobbyCountdownSeconds(int32 NewTime);
    void SetRemainingMatchEndTimeSeconds(int32 NewTime);
    void SetScoreLimit(int32 NewLimit);
    void SetRoundWinsToWin(int32 NewCount);
    void SetRoundState(EPvPARoundState NewState);
    void SetMatchPhase(EPvPAMatchPhase NewPhase);
    void SetRoundWinner(APvPArenaPlayerState* NewWinner);
    void SetMatchWinner(APvPArenaPlayerState* NewWinner);

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Match")
    int32 RemainingRoundTimeSeconds = 180;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Match")
    int32 RemainingRoundEndTimeSeconds = 0;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Match")
    int32 RemainingLobbyCountdownSeconds = 0;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Match")
    int32 RemainingMatchEndTimeSeconds = 0;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Match")
    int32 ScoreLimit = 5;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Match")
    int32 RoundWinsToWin = 2;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Match")
    EPvPARoundState RoundState = EPvPARoundState::Playing;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Match")
    EPvPAMatchPhase MatchPhase = EPvPAMatchPhase::Lobby;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Match")
    TObjectPtr<APvPArenaPlayerState> RoundWinner;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Match")
    TObjectPtr<APvPArenaPlayerState> MatchWinner;
};
