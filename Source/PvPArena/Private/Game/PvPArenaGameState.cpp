#include "Game/PvPArenaGameState.h"

#include "Game/PvPArenaPlayerState.h"
#include "Net/UnrealNetwork.h"

void APvPArenaGameState::SetRemainingRoundTimeSeconds(int32 NewTime)
{
    RemainingRoundTimeSeconds = FMath::Max(0, NewTime);
}

void APvPArenaGameState::SetRemainingRoundEndTimeSeconds(int32 NewTime)
{
    RemainingRoundEndTimeSeconds = FMath::Max(0, NewTime);
}

void APvPArenaGameState::SetRemainingLobbyCountdownSeconds(int32 NewTime)
{
    RemainingLobbyCountdownSeconds = FMath::Max(0, NewTime);
}

void APvPArenaGameState::SetRemainingMatchEndTimeSeconds(int32 NewTime)
{
    RemainingMatchEndTimeSeconds = FMath::Max(0, NewTime);
}

void APvPArenaGameState::SetScoreLimit(int32 NewLimit)
{
    ScoreLimit = FMath::Max(1, NewLimit);
}

void APvPArenaGameState::SetRoundWinsToWin(int32 NewCount)
{
    RoundWinsToWin = FMath::Max(1, NewCount);
}

void APvPArenaGameState::SetRoundState(EPvPARoundState NewState)
{
    RoundState = NewState;
}

void APvPArenaGameState::SetMatchPhase(EPvPAMatchPhase NewPhase)
{
    MatchPhase = NewPhase;
}

void APvPArenaGameState::SetRoundWinner(APvPArenaPlayerState* NewWinner)
{
    RoundWinner = NewWinner;
}

void APvPArenaGameState::SetMatchWinner(APvPArenaPlayerState* NewWinner)
{
    MatchWinner = NewWinner;
}

void APvPArenaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APvPArenaGameState, RemainingRoundTimeSeconds);
    DOREPLIFETIME(APvPArenaGameState, RemainingRoundEndTimeSeconds);
    DOREPLIFETIME(APvPArenaGameState, RemainingLobbyCountdownSeconds);
    DOREPLIFETIME(APvPArenaGameState, RemainingMatchEndTimeSeconds);
    DOREPLIFETIME(APvPArenaGameState, ScoreLimit);
    DOREPLIFETIME(APvPArenaGameState, RoundWinsToWin);
    DOREPLIFETIME(APvPArenaGameState, RoundState);
    DOREPLIFETIME(APvPArenaGameState, MatchPhase);
    DOREPLIFETIME(APvPArenaGameState, RoundWinner);
    DOREPLIFETIME(APvPArenaGameState, MatchWinner);
}
