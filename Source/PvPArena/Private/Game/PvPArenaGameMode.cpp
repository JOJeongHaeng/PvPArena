#include "Game/PvPArenaGameMode.h"

#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "Player/PvPArenaPlayerState.h"
#include "TimerManager.h"

APvPArenaGameMode::APvPArenaGameMode()
{
    GameStateClass = APvPArenaGameState::StaticClass();
    PlayerStateClass = APvPArenaPlayerState::StaticClass();

    LobbyDurationSeconds = 30;
    CombatDurationSeconds = 360;
    ResultDurationSeconds = 20;
    KillLimit = 30;
}

void APvPArenaGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    APvPArenaPlayerState* PvPPlayerState = NewPlayer ? Cast<APvPArenaPlayerState>(NewPlayer->PlayerState) : nullptr;
    if (!PvPPlayerState)
    {
        return;
    }

    const int32 Team0Count = CountPlayersOnTeam(0);
    const int32 Team1Count = CountPlayersOnTeam(1);
    const int32 AutoAssignedTeam = (Team0Count <= Team1Count) ? 0 : 1;

    TryAssignTeam(PvPPlayerState, AutoAssignedTeam);
}

void APvPArenaGameMode::BeginPlay()
{
    Super::BeginPlay();

    StartPhase(EPvPArenaMatchPhase::Lobby, LobbyDurationSeconds);
}

bool APvPArenaGameMode::TryAssignTeam(APvPArenaPlayerState* InPlayerState, int32 DesiredTeamId)
{
    if (!InPlayerState || !IsLobbyPhase())
    {
        return false;
    }

    InPlayerState->SetTeamId(DesiredTeamId);
    return true;
}

bool APvPArenaGameMode::IsLobbyPhase() const
{
    const APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>();
    return PvPGameState && PvPGameState->MatchPhase == EPvPArenaMatchPhase::Lobby;
}

int32 APvPArenaGameMode::CountPlayersOnTeam(int32 TeamId) const
{
    const AGameStateBase* BaseGameState = GameState;
    if (!BaseGameState)
    {
        return 0;
    }

    int32 TeamCount = 0;
    for (APlayerState* PlayerState : BaseGameState->PlayerArray)
    {
        const APvPArenaPlayerState* PvPPlayerState = Cast<APvPArenaPlayerState>(PlayerState);
        if (PvPPlayerState && PvPPlayerState->TeamId == TeamId)
        {
            ++TeamCount;
        }
    }

    return TeamCount;
}

void APvPArenaGameMode::StartPhase(EPvPArenaMatchPhase NewPhase, int32 DurationSeconds)
{
    APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>();
    if (!PvPGameState)
    {
        return;
    }

    if (NewPhase == EPvPArenaMatchPhase::Lobby)
    {
        PvPGameState->SetMatchEndReason(EPvPArenaMatchEndReason::None);
        PvPGameState->ResetTeamScores();
    }
    else if (NewPhase == EPvPArenaMatchPhase::Combat)
    {
        PvPGameState->SetMatchEndReason(EPvPArenaMatchEndReason::None);
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

    if (PvPGameState->MatchPhase == EPvPArenaMatchPhase::Combat && EvaluateEndConditions())
    {
        return;
    }

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

bool APvPArenaGameMode::EvaluateEndConditions()
{
    APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>();
    if (!PvPGameState || PvPGameState->MatchPhase != EPvPArenaMatchPhase::Combat)
    {
        return false;
    }

    if (PvPGameState->TeamAScore >= KillLimit || PvPGameState->TeamBScore >= KillLimit)
    {
        PvPGameState->SetMatchEndReason(EPvPArenaMatchEndReason::KillLimit);
        StartPhase(EPvPArenaMatchPhase::Result, ResultDurationSeconds);
        return true;
    }

    if (PvPGameState->RemainingSeconds <= 0)
    {
        PvPGameState->SetMatchEndReason(EPvPArenaMatchEndReason::TimeUp);
        StartPhase(EPvPArenaMatchPhase::Result, ResultDurationSeconds);
        return true;
    }

    return false;
}
