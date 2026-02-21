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

UCLASS()
class PVPARENA_API APvPArenaGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    APvPArenaGameState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void SetMatchPhase(EPvPArenaMatchPhase NewPhase);
    void SetRemainingSeconds(int32 NewRemainingSeconds);
    void AddTeamScore(int32 TeamId, int32 Delta);

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    EPvPArenaMatchPhase MatchPhase;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    int32 RemainingSeconds;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Score")
    int32 TeamAScore;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Score")
    int32 TeamBScore;
};
