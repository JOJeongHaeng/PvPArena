#include "Game/PvPArenaGameMode.h"

#include "GameFramework/Controller.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Pawn.h"
#include "Game/PvPArenaPlayerState.h"
#include "TimerManager.h"

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

    StartRoundTimer();
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
            GetWorldTimerManager().ClearTimer(RoundTimerHandle);
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

void APvPArenaGameMode::StartRoundTimer()
{
    GetWorldTimerManager().ClearTimer(RoundTimerHandle);
    GetWorldTimerManager().SetTimer(RoundTimerHandle, this, &APvPArenaGameMode::OnRoundSecondElapsed, 1.0f, true);
}

void APvPArenaGameMode::OnRoundSecondElapsed()
{
    APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>();
    if (!PvPGameState || bHasWinner)
    {
        GetWorldTimerManager().ClearTimer(RoundTimerHandle);
        return;
    }

    const int32 NewTime = PvPGameState->GetRemainingRoundTimeSeconds() - 1;
    PvPGameState->SetRemainingRoundTimeSeconds(NewTime);
    if (NewTime > 0)
    {
        return;
    }

    int32 ScoreA = 0;
    int32 ScoreB = 0;
    if (AGameStateBase* BaseGameState = GameState)
    {
        int32 Index = 0;
        for (APlayerState* PlayerState : BaseGameState->PlayerArray)
        {
            if (const APvPArenaPlayerState* PvPState = Cast<APvPArenaPlayerState>(PlayerState))
            {
                if (Index == 0)
                {
                    ScoreA = PvPState->GetKills();
                }
                else if (Index == 1)
                {
                    ScoreB = PvPState->GetKills();
                }
                ++Index;
            }
        }
    }

    const EPvPARoundState TimeoutState = ResolveRoundTimeout(ScoreA, ScoreB);
    PvPGameState->SetRoundState(TimeoutState);
    GetWorldTimerManager().ClearTimer(RoundTimerHandle);
}

void APvPArenaGameMode::HandlePlayerEliminated(AController* VictimController, AController* KillerController)
{
    APvPArenaPlayerState* VictimState = VictimController ? Cast<APvPArenaPlayerState>(VictimController->PlayerState) : nullptr;
    APvPArenaPlayerState* KillerState = KillerController ? Cast<APvPArenaPlayerState>(KillerController->PlayerState) : nullptr;

    RegisterKill(KillerState, VictimState);

    if (!VictimController || bHasWinner)
    {
        return;
    }

    if (APawn* VictimPawn = VictimController->GetPawn())
    {
        VictimPawn->Destroy();
    }

    TWeakObjectPtr<AController> VictimControllerWeak = VictimController;
    FTimerDelegate RespawnDelegate;
    RespawnDelegate.BindLambda([this, VictimControllerWeak]()
    {
        if (!VictimControllerWeak.IsValid())
        {
            return;
        }

        RestartPlayer(VictimControllerWeak.Get());
    });

    FTimerHandle RespawnTimerHandle;
    GetWorldTimerManager().SetTimer(RespawnTimerHandle, RespawnDelegate, static_cast<float>(RespawnDelaySeconds), false);
}
