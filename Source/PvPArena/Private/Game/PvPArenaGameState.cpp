#include "Game/PvPArenaGameState.h"

#include "Net/UnrealNetwork.h"

APvPArenaGameState::APvPArenaGameState()
{
    MatchPhase = EPvPArenaMatchPhase::Lobby;
    RemainingSeconds = 30;
    TeamAScore = 0;
    TeamBScore = 0;
    MatchEndReason = EPvPArenaMatchEndReason::None;
}

void APvPArenaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APvPArenaGameState, MatchPhase);
    DOREPLIFETIME(APvPArenaGameState, RemainingSeconds);
    DOREPLIFETIME(APvPArenaGameState, TeamAScore);
    DOREPLIFETIME(APvPArenaGameState, TeamBScore);
    DOREPLIFETIME(APvPArenaGameState, MatchEndReason);
}

void APvPArenaGameState::SetMatchPhase(EPvPArenaMatchPhase NewPhase)
{
    MatchPhase = NewPhase;
}

void APvPArenaGameState::SetRemainingSeconds(int32 NewRemainingSeconds)
{
    RemainingSeconds = FMath::Max(0, NewRemainingSeconds);
}

void APvPArenaGameState::SetMatchEndReason(EPvPArenaMatchEndReason NewEndReason)
{
    MatchEndReason = NewEndReason;
}

EPvPArenaMatchEndReason APvPArenaGameState::GetMatchEndReason() const
{
    return MatchEndReason;
}

void APvPArenaGameState::AddTeamScore(int32 TeamId, int32 Delta)
{
    if (TeamId == 0)
    {
        TeamAScore += Delta;
    }
    else if (TeamId == 1)
    {
        TeamBScore += Delta;
    }
}

void APvPArenaGameState::ResetTeamScores()
{
    TeamAScore = 0;
    TeamBScore = 0;
}
