#include "Game/PvPArenaGameMode.h"

#include "Game/PvPArenaPlayerState.h"

APvPArenaGameMode::APvPArenaGameMode()
{
    GameStateClass = APvPArenaGameState::StaticClass();
    PlayerStateClass = APvPArenaPlayerState::StaticClass();
}

void APvPArenaGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>())
    {
        PvPGameState->SetScoreLimit(ScoreLimit);
        PvPGameState->SetRemainingRoundTimeSeconds(RoundDurationSeconds);
        PvPGameState->SetRoundState(EPvPARoundState::Playing);
    }
}

void APvPArenaGameMode::RegisterKill(APvPArenaPlayerState* Killer, APvPArenaPlayerState* Victim)
{
    if (bHasWinner)
    {
        return;
    }

    if (Victim)
    {
        Victim->AddDeath();
    }

    if (Killer)
    {
        Killer->AddKill();
        if (Killer->GetKills() >= ScoreLimit)
        {
            bHasWinner = true;
            if (APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>())
            {
                PvPGameState->SetRoundState(EPvPARoundState::RoundEnd);
            }
        }
    }

    (void)RespawnDelaySeconds;
}

EPvPARoundState APvPArenaGameMode::ResolveRoundTimeout(int32 PlayerOneScore, int32 PlayerTwoScore)
{
    if (PlayerOneScore == PlayerTwoScore)
    {
        bHasWinner = false;
        return EPvPARoundState::SuddenDeath;
    }

    bHasWinner = true;
    return EPvPARoundState::RoundEnd;
}
