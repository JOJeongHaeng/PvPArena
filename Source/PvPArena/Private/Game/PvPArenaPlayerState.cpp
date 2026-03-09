#include "Game/PvPArenaPlayerState.h"

#include "Net/UnrealNetwork.h"

void APvPArenaPlayerState::AddKill()
{
    ++RoundKills;
    ++MatchKills;
}

void APvPArenaPlayerState::AddDeath()
{
    ++RoundDeaths;
    ++MatchDeaths;
}

void APvPArenaPlayerState::ResetRoundStats()
{
    RoundKills = 0;
    RoundDeaths = 0;
}

void APvPArenaPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APvPArenaPlayerState, RoundKills);
    DOREPLIFETIME(APvPArenaPlayerState, RoundDeaths);
    DOREPLIFETIME(APvPArenaPlayerState, MatchKills);
    DOREPLIFETIME(APvPArenaPlayerState, MatchDeaths);
}
