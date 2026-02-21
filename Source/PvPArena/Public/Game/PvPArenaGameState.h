#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PvPArenaGameState.generated.h"

UENUM(BlueprintType)
enum class EPvPArenaMatchPhase : uint8
{
    Lobby,
    Combat,
    Result
};

UENUM(BlueprintType)
enum class EPvPArenaMatchEndReason : uint8
{
    None,
    KillLimit,
    TimeUp
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPvPArenaMatchPhaseChangedSignature, EPvPArenaMatchPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPvPArenaRemainingSecondsChangedSignature, int32, NewRemainingSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPvPArenaTeamScoreChangedSignature, int32, TeamAScore, int32, TeamBScore);

UCLASS()
class PVPARENA_API APvPArenaGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    APvPArenaGameState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void SetMatchPhase(EPvPArenaMatchPhase NewPhase);
    void SetRemainingSeconds(int32 NewRemainingSeconds);
    void SetMatchEndReason(EPvPArenaMatchEndReason NewEndReason);
    UFUNCTION(BlueprintPure, Category = "Match")
    EPvPArenaMatchEndReason GetMatchEndReason() const;
    UFUNCTION(BlueprintCallable, Category = "Score")
    void AddTeamScore(int32 TeamId, int32 Delta);
    UFUNCTION(BlueprintCallable, Category = "Score")
    void ResetTeamScores();

    UFUNCTION()
    void OnRep_MatchPhase();

    UFUNCTION()
    void OnRep_RemainingSeconds();

    UFUNCTION()
    void OnRep_TeamAScore();

    UFUNCTION()
    void OnRep_TeamBScore();

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FPvPArenaMatchPhaseChangedSignature OnMatchPhaseChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FPvPArenaRemainingSecondsChangedSignature OnRemainingSecondsChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FPvPArenaTeamScoreChangedSignature OnTeamScoreChanged;

    UPROPERTY(ReplicatedUsing = OnRep_MatchPhase, BlueprintReadOnly, Category = "Match")
    EPvPArenaMatchPhase MatchPhase;

    UPROPERTY(ReplicatedUsing = OnRep_RemainingSeconds, BlueprintReadOnly, Category = "Match")
    int32 RemainingSeconds;

    UPROPERTY(ReplicatedUsing = OnRep_TeamAScore, BlueprintReadOnly, Category = "Score")
    int32 TeamAScore;

    UPROPERTY(ReplicatedUsing = OnRep_TeamBScore, BlueprintReadOnly, Category = "Score")
    int32 TeamBScore;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    EPvPArenaMatchEndReason MatchEndReason;
};
