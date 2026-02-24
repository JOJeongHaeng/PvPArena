#include "Game/PvPArenaPlayerState.h"

#include "Net/UnrealNetwork.h"

void APvPArenaPlayerState::AddKill()
{
    ++Kills;
}

void APvPArenaPlayerState::AddDeath()
{
    ++Deaths;
}

void APvPArenaPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APvPArenaPlayerState, Kills);
    DOREPLIFETIME(APvPArenaPlayerState, Deaths);
}
