#include "Game/PvPArenaPlayerState.h"

#include "Net/UnrealNetwork.h"

FString APvPArenaPlayerState::BuildNormalizedDisplayNickname(const FString& RawNickname)
{
    FString NormalizedNickname = RawNickname;
    NormalizedNickname.TrimStartAndEndInline();
    return NormalizedNickname;
}

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

void APvPArenaPlayerState::AddRoundWin()
{
    ++RoundWins;
}

void APvPArenaPlayerState::SetReadyForLobbyStart(bool bNewReady)
{
    bReadyForLobbyStart = bNewReady;
}

void APvPArenaPlayerState::SetDisplayNickname(const FString& NewNickname)
{
    DisplayNickname = BuildNormalizedDisplayNickname(NewNickname);
}

void APvPArenaPlayerState::ResetRoundStats()
{
    RoundKills = 0;
    RoundDeaths = 0;
}

void APvPArenaPlayerState::ResetMatchStats()
{
    ResetRoundStats();
    MatchKills = 0;
    MatchDeaths = 0;
    RoundWins = 0;
    bReadyForLobbyStart = false;
}

void APvPArenaPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APvPArenaPlayerState, RoundKills);
    DOREPLIFETIME(APvPArenaPlayerState, RoundDeaths);
    DOREPLIFETIME(APvPArenaPlayerState, MatchKills);
    DOREPLIFETIME(APvPArenaPlayerState, MatchDeaths);
    DOREPLIFETIME(APvPArenaPlayerState, RoundWins);
    DOREPLIFETIME(APvPArenaPlayerState, bReadyForLobbyStart);
    DOREPLIFETIME(APvPArenaPlayerState, DisplayNickname);
}
