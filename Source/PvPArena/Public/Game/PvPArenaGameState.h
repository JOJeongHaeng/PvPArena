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

UCLASS()
class PVPARENA_API APvPArenaGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    int32 GetRemainingRoundTimeSeconds() const { return RemainingRoundTimeSeconds; }
    int32 GetScoreLimit() const { return ScoreLimit; }
    EPvPARoundState GetRoundState() const { return RoundState; }

    void SetRemainingRoundTimeSeconds(int32 NewTime);
    void SetScoreLimit(int32 NewLimit);
    void SetRoundState(EPvPARoundState NewState);

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Match")
    int32 RemainingRoundTimeSeconds = 180;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Match")
    int32 ScoreLimit = 5;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Match")
    EPvPARoundState RoundState = EPvPARoundState::Playing;
};
