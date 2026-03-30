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

    EnterLobbyPhase(true);
}

void APvPArenaGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
}

void APvPArenaGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    if (CountConnectedPlayers() < MinimumPlayersToStartMatch)
    {
        EnterLobbyPhase(true);
    }
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
    const APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>();
    if (bHasWinner || !PvPGameState || PvPGameState->GetMatchPhase() != EPvPAMatchPhase::Playing)
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

    const EPvPARoundState CurrentRoundState = PvPGameState ? PvPGameState->GetRoundState() : EPvPARoundState::Playing;
    const int32 KillerKills = Killer ? Killer->GetKills() : 0;
    if (Killer && ShouldEndRoundOnKill(CurrentRoundState, KillerKills))
    {
        bHasWinner = true;
        AwardRoundWin(Killer);
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

bool APvPArenaGameMode::ShouldEndMatchOnRoundWin(int32 RoundWins) const
{
    return RoundWins >= IterationRoundWinsToWinDefault;
}

EPvPAMatchPhase APvPArenaGameMode::ResolveMatchPhaseAfterRoundWin(int32 RoundWins) const
{
    return ShouldEndMatchOnRoundWin(RoundWins) ? EPvPAMatchPhase::MatchEnd : EPvPAMatchPhase::Playing;
}

bool APvPArenaGameMode::ShouldContinueToNextRoundAfterRoundWin(int32 RoundWins) const
{
    return ResolveMatchPhaseAfterRoundWin(RoundWins) == EPvPAMatchPhase::Playing;
}

bool APvPArenaGameMode::IsReadyToStartMatch(int32 ConnectedPlayers, int32 ReadyPlayers) const
{
    return ConnectedPlayers >= MinimumPlayersToStartMatch;
}

bool APvPArenaGameMode::CanLobbyHostStartMatch(bool bRequestingControllerHasAuthority, int32 ConnectedPlayers) const
{
    return bRequestingControllerHasAuthority && ConnectedPlayers >= MinimumPlayersToStartMatch;
}

bool APvPArenaGameMode::ShouldReturnToLobbyAfterMatchEnd(int32 RemainingMatchEndSeconds) const
{
    return RemainingMatchEndSeconds <= 0;
}

void APvPArenaGameMode::RequestLobbyMatchStart(AController* RequestingController)
{
    APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>();
    if (!PvPGameState || PvPGameState->GetMatchPhase() != EPvPAMatchPhase::Lobby)
    {
        return;
    }

    if (!CanLobbyHostStartMatch(RequestingController && RequestingController->HasAuthority(), CountConnectedPlayers()))
    {
        return;
    }

    StartMatchFlow();
}

void APvPArenaGameMode::HandleLobbyReadyStateChanged(APvPArenaPlayerState* PlayerState, bool bReadyForStart)
{
    if (!PlayerState)
    {
        return;
    }

    PlayerState->SetReadyForLobbyStart(bReadyForStart);
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

AActor* APvPArenaGameMode::ChooseRoundStartFromCandidates(const TArray<AActor*>& CandidateStarts, AController* Player, const TSet<TObjectKey<AActor>>& UsedStarts) const
{
    if (CandidateStarts.IsEmpty())
    {
        return nullptr;
    }

    TArray<AActor*> EligibleStarts;
    EligibleStarts.Reserve(CandidateStarts.Num());

    for (AActor* CandidateStart : CandidateStarts)
    {
        if (!CandidateStart || UsedStarts.Contains(CandidateStart))
        {
            continue;
        }

        EligibleStarts.Add(CandidateStart);
    }

    const TArray<AActor*>& StartsToUse = EligibleStarts.IsEmpty() ? CandidateStarts : EligibleStarts;
    const TObjectKey<AController> PlayerKey(Player);
    const TWeakObjectPtr<AActor>* PreviousStart = LastChosenPlayerStartsByController.Find(PlayerKey);
    return ChooseRespawnStartFromCandidates(StartsToUse, PreviousStart ? PreviousStart->Get() : nullptr);
}

void APvPArenaGameMode::BeginRoundEndPhase()
{
    GetWorldTimerManager().ClearTimer(RoundTimerHandle);
    GetWorldTimerManager().ClearTimer(RoundResetTimerHandle);
    GetWorldTimerManager().ClearTimer(MatchEndTimerHandle);

    if (APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>())
    {
        PvPGameState->SetMatchPhase(EPvPAMatchPhase::Playing);
        PvPGameState->SetRoundState(EPvPARoundState::RoundEnd);
        PvPGameState->SetRemainingRoundTimeSeconds(0);
        PvPGameState->SetRemainingRoundEndTimeSeconds(RoundEndDelaySeconds);
        PvPGameState->SetRemainingMatchEndTimeSeconds(0);
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
        PvPGameState->SetMatchPhase(EPvPAMatchPhase::Playing);
        PvPGameState->SetRoundState(EPvPARoundState::Playing);
        PvPGameState->SetRemainingRoundTimeSeconds(RoundDurationSeconds);
        PvPGameState->SetRemainingRoundEndTimeSeconds(0);
        PvPGameState->SetRemainingMatchEndTimeSeconds(0);
        PvPGameState->SetRoundWinner(nullptr);
    }

    StartRoundTimer();
}

void APvPArenaGameMode::ResetAllPlayersForNextRound()
{
    if (!GetWorld())
    {
        return;
    }

    TSet<TObjectKey<AActor>> UsedRoundStartSpots;
    TArray<AActor*> CandidateStarts;
    UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), CandidateStarts);

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

        if (AActor* RoundStart = ChooseRoundStartFromCandidates(CandidateStarts, Controller, UsedRoundStartSpots))
        {
            UsedRoundStartSpots.Add(RoundStart);
            RestartPlayerAtPlayerStart(Controller, RoundStart);
        }
        else
        {
            RestartPlayer(Controller);
        }

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
    if (!PvPGameState || bHasWinner || PvPGameState->GetMatchPhase() != EPvPAMatchPhase::Playing)
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
        AwardRoundWin(ResolveRoundWinnerFromScores());
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

void APvPArenaGameMode::EnterLobbyPhase(bool bResetMatchStats)
{
    bHasWinner = false;
    GetWorldTimerManager().ClearTimer(RoundTimerHandle);
    GetWorldTimerManager().ClearTimer(RoundResetTimerHandle);
    GetWorldTimerManager().ClearTimer(LobbyCountdownTimerHandle);
    GetWorldTimerManager().ClearTimer(MatchEndTimerHandle);

    if (bResetMatchStats)
    {
        ResetAllMatchStats();
    }

    if (APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>())
    {
        PvPGameState->SetScoreLimit(ScoreLimit);
        PvPGameState->SetRoundWinsToWin(IterationRoundWinsToWinDefault);
        PvPGameState->SetMatchPhase(EPvPAMatchPhase::Lobby);
        PvPGameState->SetRoundState(EPvPARoundState::Playing);
        PvPGameState->SetRemainingRoundTimeSeconds(0);
        PvPGameState->SetRemainingRoundEndTimeSeconds(0);
        PvPGameState->SetRemainingLobbyCountdownSeconds(0);
        PvPGameState->SetRemainingMatchEndTimeSeconds(0);
        PvPGameState->SetRoundWinner(nullptr);
        PvPGameState->SetMatchWinner(nullptr);
    }
}

void APvPArenaGameMode::TryStartMatchFromLobby()
{
    CancelLobbyCountdown();
}

void APvPArenaGameMode::CancelLobbyCountdown()
{
    GetWorldTimerManager().ClearTimer(LobbyCountdownTimerHandle);

    if (APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>())
    {
        PvPGameState->SetRemainingLobbyCountdownSeconds(0);
    }
}

void APvPArenaGameMode::OnLobbyCountdownSecondElapsed()
{
    CancelLobbyCountdown();
}

void APvPArenaGameMode::StartMatchFlow()
{
    bHasWinner = false;
    GetWorldTimerManager().ClearTimer(RoundTimerHandle);
    GetWorldTimerManager().ClearTimer(RoundResetTimerHandle);
    GetWorldTimerManager().ClearTimer(LobbyCountdownTimerHandle);
    GetWorldTimerManager().ClearTimer(MatchEndTimerHandle);
    ResetAllMatchStats();

    if (APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>())
    {
        PvPGameState->SetScoreLimit(ScoreLimit);
        PvPGameState->SetRoundWinsToWin(IterationRoundWinsToWinDefault);
        PvPGameState->SetMatchPhase(EPvPAMatchPhase::Playing);
        PvPGameState->SetRoundState(EPvPARoundState::Playing);
        PvPGameState->SetRemainingRoundTimeSeconds(RoundDurationSeconds);
        PvPGameState->SetRemainingRoundEndTimeSeconds(0);
        PvPGameState->SetRemainingLobbyCountdownSeconds(0);
        PvPGameState->SetRemainingMatchEndTimeSeconds(0);
        PvPGameState->SetRoundWinner(nullptr);
        PvPGameState->SetMatchWinner(nullptr);
    }

    ResetAllPlayersForNextRound();
    StartRoundTimer();
}

void APvPArenaGameMode::BeginMatchEndPhase(APvPArenaPlayerState* MatchWinner)
{
    bHasWinner = true;
    GetWorldTimerManager().ClearTimer(RoundTimerHandle);
    GetWorldTimerManager().ClearTimer(RoundResetTimerHandle);
    GetWorldTimerManager().ClearTimer(MatchEndTimerHandle);

    if (APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>())
    {
        PvPGameState->SetMatchPhase(EPvPAMatchPhase::MatchEnd);
        PvPGameState->SetRoundState(EPvPARoundState::RoundEnd);
        PvPGameState->SetRemainingRoundTimeSeconds(0);
        PvPGameState->SetRemainingRoundEndTimeSeconds(0);
        PvPGameState->SetRemainingMatchEndTimeSeconds(MatchEndDelaySeconds);
        PvPGameState->SetMatchWinner(MatchWinner);
    }

    GetWorldTimerManager().SetTimer(
        MatchEndTimerHandle,
        this,
        &APvPArenaGameMode::OnMatchEndSecondElapsed,
        1.0f,
        true);
}

void APvPArenaGameMode::AwardRoundWin(APvPArenaPlayerState* RoundWinner)
{
    if (!RoundWinner)
    {
        BeginRoundEndPhase();
        return;
    }

    RoundWinner->AddRoundWin();

    if (APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>())
    {
        PvPGameState->SetRoundWinner(RoundWinner);
    }

    if (ShouldEndMatchOnRoundWin(RoundWinner->GetRoundWins()))
    {
        BeginMatchEndPhase(RoundWinner);
        return;
    }

    BeginRoundEndPhase();
}

APvPArenaPlayerState* APvPArenaGameMode::ResolveRoundWinnerFromScores() const
{
    const AGameStateBase* BaseGameState = GameState;
    if (!BaseGameState)
    {
        return nullptr;
    }

    APvPArenaPlayerState* BestPlayerState = nullptr;
    int32 BestKills = TNumericLimits<int32>::Min();
    bool bTie = false;

    for (APlayerState* PlayerState : BaseGameState->PlayerArray)
    {
        APvPArenaPlayerState* PvPState = Cast<APvPArenaPlayerState>(PlayerState);
        if (!PvPState)
        {
            continue;
        }

        const int32 RoundKills = PvPState->GetRoundKills();
        if (RoundKills > BestKills)
        {
            BestKills = RoundKills;
            BestPlayerState = PvPState;
            bTie = false;
        }
        else if (RoundKills == BestKills)
        {
            bTie = true;
        }
    }

    return bTie ? nullptr : BestPlayerState;
}

int32 APvPArenaGameMode::CountConnectedPlayers() const
{
    const AGameStateBase* BaseGameState = GameState;
    if (!BaseGameState)
    {
        return 0;
    }

    int32 ConnectedPlayers = 0;
    for (APlayerState* PlayerState : BaseGameState->PlayerArray)
    {
        if (PlayerState)
        {
            ++ConnectedPlayers;
        }
    }

    return ConnectedPlayers;
}

int32 APvPArenaGameMode::CountReadyPlayers() const
{
    const AGameStateBase* BaseGameState = GameState;
    if (!BaseGameState)
    {
        return 0;
    }

    int32 ReadyPlayers = 0;
    for (APlayerState* PlayerState : BaseGameState->PlayerArray)
    {
        const APvPArenaPlayerState* PvPPlayerState = Cast<APvPArenaPlayerState>(PlayerState);
        if (PvPPlayerState && PvPPlayerState->IsReadyForLobbyStart())
        {
            ++ReadyPlayers;
        }
    }

    return ReadyPlayers;
}

void APvPArenaGameMode::ResetAllMatchStats()
{
    if (AGameStateBase* BaseGameState = GameState)
    {
        for (APlayerState* PlayerState : BaseGameState->PlayerArray)
        {
            if (APvPArenaPlayerState* PvPPlayerState = Cast<APvPArenaPlayerState>(PlayerState))
            {
                PvPPlayerState->ResetMatchStats();
            }
        }
    }
}

void APvPArenaGameMode::OnMatchEndSecondElapsed()
{
    APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>();
    if (!PvPGameState)
    {
        GetWorldTimerManager().ClearTimer(MatchEndTimerHandle);
        return;
    }

    const int32 NewTime = PvPGameState->GetRemainingMatchEndTimeSeconds() - 1;
    PvPGameState->SetRemainingMatchEndTimeSeconds(NewTime);
    if (NewTime > 0)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(MatchEndTimerHandle);
    EnterLobbyPhase(true);
}
