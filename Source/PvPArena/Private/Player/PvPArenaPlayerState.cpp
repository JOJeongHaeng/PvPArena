#include "Player/PvPArenaPlayerState.h"

#include "Net/UnrealNetwork.h"

APvPArenaPlayerState::APvPArenaPlayerState()
{
    TeamId = 0;
    Kills = 0;
    Deaths = 0;
    bIsDead = false;
    RespawnRemainingSeconds = 0;
}

void APvPArenaPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APvPArenaPlayerState, TeamId);
    DOREPLIFETIME(APvPArenaPlayerState, Kills);
    DOREPLIFETIME(APvPArenaPlayerState, Deaths);
    DOREPLIFETIME(APvPArenaPlayerState, bIsDead);
    DOREPLIFETIME(APvPArenaPlayerState, RespawnRemainingSeconds);
}

void APvPArenaPlayerState::SetTeamId(int32 NewTeamId)
{
    TeamId = FMath::Clamp(NewTeamId, 0, 1);
}

void APvPArenaPlayerState::AddKill()
{
    ++Kills;
}

void APvPArenaPlayerState::AddDeath()
{
    ++Deaths;
}

void APvPArenaPlayerState::SetDeadState(bool bNewIsDead, int32 NewRespawnRemainingSeconds)
{
    bIsDead = bNewIsDead;
    RespawnRemainingSeconds = FMath::Max(0, NewRespawnRemainingSeconds);
}
