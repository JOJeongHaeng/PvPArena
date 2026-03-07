#include "Game/PvPArenaGameState.h"

#include "Net/UnrealNetwork.h"

void APvPArenaGameState::SetRemainingRoundTimeSeconds(int32 NewTime)
{
    RemainingRoundTimeSeconds = FMath::Max(0, NewTime);
}

void APvPArenaGameState::SetRemainingRoundEndTimeSeconds(int32 NewTime)
{
    RemainingRoundEndTimeSeconds = FMath::Max(0, NewTime);
}

void APvPArenaGameState::SetScoreLimit(int32 NewLimit)
{
    ScoreLimit = FMath::Max(1, NewLimit);
}

void APvPArenaGameState::SetRoundState(EPvPARoundState NewState)
{
    RoundState = NewState;
}

void APvPArenaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APvPArenaGameState, RemainingRoundTimeSeconds);
    DOREPLIFETIME(APvPArenaGameState, RemainingRoundEndTimeSeconds);
    DOREPLIFETIME(APvPArenaGameState, ScoreLimit);
    DOREPLIFETIME(APvPArenaGameState, RoundState);
}
