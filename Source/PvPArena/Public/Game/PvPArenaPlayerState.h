#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PvPArenaPlayerState.generated.h"

UCLASS()
class PVPARENA_API APvPArenaPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    int32 GetRoundKills() const { return RoundKills; }
    int32 GetRoundDeaths() const { return RoundDeaths; }
    int32 GetMatchKills() const { return MatchKills; }
    int32 GetMatchDeaths() const { return MatchDeaths; }
    int32 GetKills() const { return GetRoundKills(); }
    int32 GetDeaths() const { return GetRoundDeaths(); }

    void AddKill();
    void AddDeath();
    void ResetRoundStats();

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Score")
    int32 RoundKills = 0;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Score")
    int32 RoundDeaths = 0;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Score")
    int32 MatchKills = 0;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Score")
    int32 MatchDeaths = 0;
};
