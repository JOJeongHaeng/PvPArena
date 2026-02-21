#include "Game/PvPArenaGameMode.h"

#include "Engine/World.h"
#include "TimerManager.h"

APvPArenaGameMode::APvPArenaGameMode()
{
    GameStateClass = APvPArenaGameState::StaticClass();

    LobbyDurationSeconds = 30;
    CombatDurationSeconds = 360;
    ResultDurationSeconds = 20;
}

void APvPArenaGameMode::BeginPlay()
{
    Super::BeginPlay();

    StartPhase(EPvPArenaMatchPhase::Lobby, LobbyDurationSeconds);
}

void APvPArenaGameMode::StartPhase(EPvPArenaMatchPhase NewPhase, int32 DurationSeconds)
{
    APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>();
    if (!PvPGameState)
    {
        return;
    }

    PvPGameState->SetMatchPhase(NewPhase);
    PvPGameState->SetRemainingSeconds(DurationSeconds);

    GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
    GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &APvPArenaGameMode::HandlePhaseTick, 1.0f, true);
}

void APvPArenaGameMode::HandlePhaseTick()
{
    APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>();
    if (!PvPGameState)
    {
        GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
        return;
    }

    const int32 NextRemaining = PvPGameState->RemainingSeconds - 1;
    PvPGameState->SetRemainingSeconds(NextRemaining);

    if (PvPGameState->RemainingSeconds > 0)
    {
        return;
    }

    switch (PvPGameState->MatchPhase)
    {
    case EPvPArenaMatchPhase::Lobby:
        StartPhase(EPvPArenaMatchPhase::Combat, CombatDurationSeconds);
        break;
    case EPvPArenaMatchPhase::Combat:
        StartPhase(EPvPArenaMatchPhase::Result, ResultDurationSeconds);
        break;
    case EPvPArenaMatchPhase::Result:
        StartPhase(EPvPArenaMatchPhase::Lobby, LobbyDurationSeconds);
        break;
    default:
        break;
    }
}
