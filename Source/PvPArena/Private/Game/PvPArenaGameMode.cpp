#include "Game/PvPArenaGameMode.h"

#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerStart.h"
#include "Player/PvPArenaCharacter.h"
#include "Game/PvPArenaPlayerController.h"
#include "Game/PvPArenaPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

APvPArenaGameMode::APvPArenaGameMode()
{
    GameStateClass = APvPArenaGameState::StaticClass();
    PlayerStateClass = APvPArenaPlayerState::StaticClass();
    PlayerControllerClass = APvPArenaPlayerController::StaticClass();
    HUDClass = AHUD::StaticClass();
}

void APvPArenaGameMode::BeginPlay()
{
    Super::BeginPlay();

    bHasWinner = false;
    GetWorldTimerManager().ClearTimer(RoundResetTimerHandle);

    if (APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>())
    {
        PvPGameState->SetScoreLimit(ScoreLimit);
        PvPGameState->SetRemainingRoundTimeSeconds(RoundDurationSeconds);
        PvPGameState->SetRemainingRoundEndTimeSeconds(0);
        PvPGameState->SetRoundState(EPvPARoundState::Playing);
    }

    StartRoundTimer();
}

AActor* APvPArenaGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    TArray<AActor*> CandidateStarts;
    UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), CandidateStarts);

    if (AActor* ChosenStart = ChooseRespawnStartForPlayer(CandidateStarts, Player))
    {
        return ChosenStart;
    }

    return Super::ChoosePlayerStart_Implementation(Player);
}

bool APvPArenaGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
    return false;
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
    }

    APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>();
    const EPvPARoundState CurrentRoundState = PvPGameState ? PvPGameState->GetRoundState() : EPvPARoundState::Playing;
    const int32 KillerKills = Killer ? Killer->GetKills() : 0;
    if (Killer && ShouldEndRoundOnKill(CurrentRoundState, KillerKills))
    {
        bHasWinner = true;
        BeginRoundEndPhase();
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

bool APvPArenaGameMode::ShouldEndRoundOnKill(EPvPARoundState CurrentRoundState, int32 KillerKills) const
{
    if (CurrentRoundState == EPvPARoundState::SuddenDeath)
    {
        return true;
    }

    return KillerKills >= ScoreLimit;
}

bool APvPArenaGameMode::ShouldScheduleRespawnAfterElimination(bool bHasVictimController) const
{
    return bHasVictimController && !bHasWinner;
}

AActor* APvPArenaGameMode::ChooseRespawnStartFromCandidates(const TArray<AActor*>& CandidateStarts, const AActor* PreviousStart) const
{
    if (CandidateStarts.IsEmpty())
    {
        return nullptr;
    }

    TArray<AActor*> EligibleStarts = CandidateStarts;
    if (EligibleStarts.Num() > 1 && PreviousStart)
    {
        EligibleStarts.RemoveSingleSwap(const_cast<AActor*>(PreviousStart));
    }

    const TArray<AActor*>& StartsToUse = EligibleStarts.IsEmpty() ? CandidateStarts : EligibleStarts;
    return StartsToUse[FMath::RandHelper(StartsToUse.Num())];
}

AActor* APvPArenaGameMode::ChooseRespawnStartForPlayer(const TArray<AActor*>& CandidateStarts, AController* Player)
{
    const TObjectKey<AController> PlayerKey(Player);
    const TWeakObjectPtr<AActor>* PreviousStart = LastChosenPlayerStartsByController.Find(PlayerKey);
    AActor* ChosenStart = ChooseRespawnStartFromCandidates(CandidateStarts, PreviousStart ? PreviousStart->Get() : nullptr);

    if (ChosenStart && Player)
    {
        LastChosenPlayerStartsByController.Add(PlayerKey, ChosenStart);
    }

    return ChosenStart;
}

void APvPArenaGameMode::BeginRoundEndPhase()
{
    GetWorldTimerManager().ClearTimer(RoundTimerHandle);
    GetWorldTimerManager().ClearTimer(RoundResetTimerHandle);

    if (APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>())
    {
        PvPGameState->SetRoundState(EPvPARoundState::RoundEnd);
        PvPGameState->SetRemainingRoundTimeSeconds(0);
        PvPGameState->SetRemainingRoundEndTimeSeconds(RoundEndDelaySeconds);
    }

    GetWorldTimerManager().SetTimer(
        RoundResetTimerHandle,
        this,
        &APvPArenaGameMode::OnRoundResetSecondElapsed,
        1.0f,
        true);
}

void APvPArenaGameMode::OnRoundResetSecondElapsed()
{
    APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>();
    if (!PvPGameState)
    {
        GetWorldTimerManager().ClearTimer(RoundResetTimerHandle);
        return;
    }

    const int32 NewRoundEndTime = PvPGameState->GetRemainingRoundEndTimeSeconds() - 1;
    PvPGameState->SetRemainingRoundEndTimeSeconds(NewRoundEndTime);
    if (NewRoundEndTime > 0)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(RoundResetTimerHandle);
    HandleRoundReset();
}

void APvPArenaGameMode::HandleRoundReset()
{
    bHasWinner = false;

    if (AGameStateBase* BaseGameState = GameState)
    {
        for (APlayerState* PlayerState : BaseGameState->PlayerArray)
        {
            if (APvPArenaPlayerState* PvPPlayerState = Cast<APvPArenaPlayerState>(PlayerState))
            {
                PvPPlayerState->ResetRoundStats();
            }
        }
    }

    ResetAllPlayersForNextRound();

    if (APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>())
    {
        PvPGameState->SetRoundState(EPvPARoundState::Playing);
        PvPGameState->SetRemainingRoundTimeSeconds(RoundDurationSeconds);
        PvPGameState->SetRemainingRoundEndTimeSeconds(0);
    }

    StartRoundTimer();
}

void APvPArenaGameMode::ResetAllPlayersForNextRound()
{
    if (!GetWorld())
    {
        return;
    }

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        AController* Controller = It->Get();
        if (!Controller)
        {
            continue;
        }

        if (APawn* ExistingPawn = Controller->GetPawn())
        {
            ExistingPawn->Destroy();
        }

        RestartPlayer(Controller);

        if (APvPArenaCharacter* RespawnedCharacter = Cast<APvPArenaCharacter>(Controller->GetPawn()))
        {
            RespawnedCharacter->SetInvulnerableForSeconds(RespawnInvulnerabilitySeconds);
        }
    }
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
    if (TimeoutState == EPvPARoundState::RoundEnd)
    {
        BeginRoundEndPhase();
        return;
    }

    GetWorldTimerManager().ClearTimer(RoundTimerHandle);
}

void APvPArenaGameMode::HandlePlayerEliminated(AController* VictimController, AController* KillerController)
{
    APvPArenaPlayerState* VictimState = VictimController ? Cast<APvPArenaPlayerState>(VictimController->PlayerState) : nullptr;
    APvPArenaPlayerState* KillerState = KillerController ? Cast<APvPArenaPlayerState>(KillerController->PlayerState) : nullptr;

    RegisterKill(KillerState, VictimState);

    if (!ShouldScheduleRespawnAfterElimination(VictimController != nullptr))
    {
        return;
    }

    if (APvPArenaCharacter* VictimCharacter = Cast<APvPArenaCharacter>(VictimController->GetPawn()))
    {
        const float DeathAnimationDuration = VictimCharacter->GetDeathAnimationDuration();
        if (DeathAnimationDuration > 0.0f)
        {
            FTimerHandle DeathCleanupTimerHandle;
            TWeakObjectPtr<APvPArenaCharacter> VictimCharacterWeak = VictimCharacter;
            GetWorldTimerManager().SetTimer(
                DeathCleanupTimerHandle,
                [VictimCharacterWeak]()
                {
                    if (VictimCharacterWeak.IsValid())
                    {
                        VictimCharacterWeak->Destroy();
                    }
                },
                DeathAnimationDuration,
                false);
        }
        else
        {
            VictimCharacter->Destroy();
        }
    }

    TWeakObjectPtr<AController> VictimControllerWeak = VictimController;
    FTimerDelegate RespawnDelegate;
    RespawnDelegate.BindLambda([this, VictimControllerWeak]()
    {
        if (!VictimControllerWeak.IsValid())
        {
            return;
        }

        if (APawn* ExistingPawn = VictimControllerWeak->GetPawn())
        {
            ExistingPawn->Destroy();
        }

        RestartPlayer(VictimControllerWeak.Get());

        if (APvPArenaCharacter* RespawnedCharacter = Cast<APvPArenaCharacter>(VictimControllerWeak->GetPawn()))
        {
            RespawnedCharacter->SetInvulnerableForSeconds(RespawnInvulnerabilitySeconds);
        }
    });

    FTimerHandle RespawnTimerHandle;
    GetWorldTimerManager().SetTimer(RespawnTimerHandle, RespawnDelegate, static_cast<float>(RespawnDelaySeconds), false);
}
