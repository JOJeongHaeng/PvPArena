#include "Game/PvPArenaGameMode.h"

#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/SpectatorPawn.h"
#include "Player/PvPArenaCharacter.h"
#include "Player/PvPArenaSpectatorPawn.h"
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
    SpectatorClass = APvPArenaSpectatorPawn::StaticClass();
}

namespace
{
bool MatchesSpawnMarker(const FString& Identifier, const TCHAR* Marker)
{
    return Identifier.Contains(Marker, ESearchCase::IgnoreCase, ESearchDir::FromStart);
}

bool IsBlueTeamSpawnIdentifier(const FString& Identifier)
{
    return MatchesSpawnMarker(Identifier, TEXT("TeamLeft"))
        || MatchesSpawnMarker(Identifier, TEXT("BluePlayerStart"))
        || MatchesSpawnMarker(Identifier, TEXT("TeamBlue"));
}

bool IsRedTeamSpawnIdentifier(const FString& Identifier)
{
    return MatchesSpawnMarker(Identifier, TEXT("TeamRight"))
        || MatchesSpawnMarker(Identifier, TEXT("RedPlayerStart"))
        || MatchesSpawnMarker(Identifier, TEXT("TeamRed"));
}

bool IsFreeForAllSpawnIdentifier(const FString& Identifier)
{
    return MatchesSpawnMarker(Identifier, TEXT("FreeForAll"))
        || MatchesSpawnMarker(Identifier, TEXT("FFA"))
        || (MatchesSpawnMarker(Identifier, TEXT("PlayerStart"))
            && !IsBlueTeamSpawnIdentifier(Identifier)
            && !IsRedTeamSpawnIdentifier(Identifier));
}

FString ResolvePlayerStartIdentifier(const APlayerStart* PlayerStart)
{
    if (!PlayerStart)
    {
        return FString();
    }

    if (!PlayerStart->PlayerStartTag.IsNone())
    {
        return PlayerStart->PlayerStartTag.ToString();
    }

    for (const FName& ActorTag : PlayerStart->Tags)
    {
        if (!ActorTag.IsNone())
        {
            return ActorTag.ToString();
        }
    }

#if WITH_EDITOR
    return PlayerStart->GetActorLabel();
#else
    return PlayerStart->GetName();
#endif
}
}

FString APvPArenaGameMode::BuildDefaultDisplayNickname(int32 PlayerIndex)
{
    return FString::Printf(TEXT("Player%d"), FMath::Max(1, PlayerIndex));
}

void APvPArenaGameMode::BeginPlay()
{
    Super::BeginPlay();

    EnterLobbyPhase(true);
}

void APvPArenaGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

    if (!ErrorMessage.IsEmpty())
    {
        return;
    }

    if (CountConnectedPlayers() >= MaximumLobbyPlayers)
    {
        ErrorMessage = TEXT("Lobby is full.");
    }
}

void APvPArenaGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    APvPArenaPlayerState* NewPlayerState = NewPlayer ? Cast<APvPArenaPlayerState>(NewPlayer->PlayerState) : nullptr;
    if (!NewPlayerState)
    {
        return;
    }

    if (NewPlayerState->GetDisplayNickname().IsEmpty())
    {
        NewPlayerState->SetDisplayNickname(ResolveUniqueDefaultDisplayNickname(NewPlayerState));
    }

    EPvPALobbyMatchMode CurrentLobbyMatchMode = EPvPALobbyMatchMode::FreeForAll;
    if (const AGameStateBase* BaseGameState = GameState)
    {
        for (APlayerState* PlayerState : BaseGameState->PlayerArray)
        {
            const APvPArenaPlayerState* ExistingPlayerState = Cast<APvPArenaPlayerState>(PlayerState);
            if (!ExistingPlayerState || ExistingPlayerState == NewPlayerState)
            {
                continue;
            }

            CurrentLobbyMatchMode = ExistingPlayerState->GetLobbyMatchMode();
            break;
        }
    }

    ApplyLobbyModeChangeToPlayerState(NewPlayerState, CurrentLobbyMatchMode);
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
    CandidateStarts = FilterPlayerStartsForPlayer(CandidateStarts, Player);

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

    if (GetLobbyMatchMode() == EPvPALobbyMatchMode::TeamVersus)
    {
        const EPvPALobbyTeam VictimTeam = Victim ? Victim->GetLobbyTeam() : EPvPALobbyTeam::None;
        if (VictimTeam == EPvPALobbyTeam::Left || VictimTeam == EPvPALobbyTeam::Right)
        {
            const int32 RemainingVictimTeamAlivePlayers = CountAlivePlayersOnLobbyTeam(VictimTeam);
            if (RemainingVictimTeamAlivePlayers <= 0)
            {
                bHasWinner = true;
                const EPvPALobbyTeam WinningTeam = VictimTeam == EPvPALobbyTeam::Left
                    ? EPvPALobbyTeam::Right
                    : EPvPALobbyTeam::Left;
                AwardRoundWinners(
                    GatherPlayersOnLobbyTeam(WinningTeam),
                    ResolveRepresentingPlayerForTeam(WinningTeam));
            }
        }
        return;
    }

    const EPvPARoundState CurrentRoundState = PvPGameState ? PvPGameState->GetRoundState() : EPvPARoundState::Playing;
    const int32 KillerKills = Killer ? Killer->GetKills() : 0;
    if (CurrentRoundState == EPvPARoundState::SuddenDeath)
    {
        if (APvPArenaPlayerState* UniqueLeader = ResolveRoundWinnerFromScores())
        {
            bHasWinner = true;
            BeginMatchEndPhase(UniqueLeader);
        }
        return;
    }

    if (Killer && ShouldEndRoundOnKill(CurrentRoundState, KillerKills))
    {
        bHasWinner = true;
        BeginMatchEndPhase(Killer);
    }

    (void)RespawnDelaySeconds;
}

EPvPARoundState APvPArenaGameMode::ResolveRoundTimeout(int32 PlayerOneScore, int32 PlayerTwoScore)
{
    bHasWinner = true;
    return ResolveFreeForAllRoundTimeoutState(PlayerOneScore == PlayerTwoScore);
}

EPvPARoundState APvPArenaGameMode::ResolveFreeForAllRoundTimeoutState(bool bHasTieForLead) const
{
    (void)bHasTieForLead;
    return EPvPARoundState::RoundEnd;
}

EPvPARoundState APvPArenaGameMode::ResolveTeamVersusRoundTimeoutState(int32 LeftTeamAlivePlayers, int32 RightTeamAlivePlayers) const
{
    (void)LeftTeamAlivePlayers;
    (void)RightTeamAlivePlayers;
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
    return ShouldEndMatchOnRoundWinState(RoundWins, false);
}

bool APvPArenaGameMode::ShouldEndMatchOnRoundWinState(int32 HighestRoundWins, bool bHasTieAtHighestRoundWins) const
{
    return HighestRoundWins >= IterationRoundWinsToWinDefault && !bHasTieAtHighestRoundWins;
}

EPvPAMatchPhase APvPArenaGameMode::ResolveMatchPhaseAfterRoundWin(int32 RoundWins) const
{
    return ResolveMatchPhaseAfterRoundWinState(RoundWins, false);
}

EPvPAMatchPhase APvPArenaGameMode::ResolveMatchPhaseAfterRoundWinState(int32 HighestRoundWins, bool bHasTieAtHighestRoundWins) const
{
    return ShouldEndMatchOnRoundWinState(HighestRoundWins, bHasTieAtHighestRoundWins)
        ? EPvPAMatchPhase::MatchEnd
        : EPvPAMatchPhase::Playing;
}

bool APvPArenaGameMode::ShouldContinueToNextRoundAfterRoundWin(int32 RoundWins) const
{
    return ResolveMatchPhaseAfterRoundWin(RoundWins) == EPvPAMatchPhase::Playing;
}

bool APvPArenaGameMode::IsReadyToStartMatch(EPvPALobbyMatchMode LobbyMatchMode, int32 ConnectedPlayers, int32 LeftTeamPlayers, int32 RightTeamPlayers) const
{
    if (ConnectedPlayers < MinimumPlayersToStartMatch || ConnectedPlayers > MaximumLobbyPlayers)
    {
        return false;
    }

    if (LobbyMatchMode == EPvPALobbyMatchMode::TeamVersus)
    {
        return LeftTeamPlayers > 0 && RightTeamPlayers > 0;
    }

    return true;
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

    const int32 ConnectedPlayers = CountConnectedPlayers();
    if (!CanLobbyHostStartMatch(RequestingController && RequestingController->HasAuthority(), ConnectedPlayers))
    {
        return;
    }

    if (!IsReadyToStartMatch(
            GetLobbyMatchMode(),
            ConnectedPlayers,
            CountPlayersOnLobbyTeam(EPvPALobbyTeam::Left),
            CountPlayersOnLobbyTeam(EPvPALobbyTeam::Right)))
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

void APvPArenaGameMode::HandleLobbyDisplayNicknameChanged(APvPArenaPlayerState* PlayerState, const FString& RequestedNickname)
{
    if (!PlayerState)
    {
        return;
    }

    const FString NormalizedNickname = APvPArenaPlayerState::BuildNormalizedDisplayNickname(RequestedNickname);
    PlayerState->SetDisplayNickname(
        NormalizedNickname.IsEmpty()
            ? ResolveUniqueDefaultDisplayNickname(PlayerState)
            : NormalizedNickname);
}

void APvPArenaGameMode::HandleLobbyMatchModeChanged(AController* RequestingController, EPvPALobbyMatchMode NewLobbyMatchMode)
{
    if (!RequestingController || !RequestingController->HasAuthority())
    {
        return;
    }

    if (AGameStateBase* BaseGameState = GameState)
    {
        for (APlayerState* PlayerState : BaseGameState->PlayerArray)
        {
            ApplyLobbyModeChangeToPlayerState(Cast<APvPArenaPlayerState>(PlayerState), NewLobbyMatchMode);
        }
    }
}

void APvPArenaGameMode::HandleLobbyTeamSelectionChanged(APvPArenaPlayerState* PlayerState, EPvPALobbyTeam NewLobbyTeam)
{
    if (!PlayerState || GetLobbyMatchMode() != EPvPALobbyMatchMode::TeamVersus)
    {
        return;
    }

    PlayerState->SetLobbyTeam(NewLobbyTeam);
}

void APvPArenaGameMode::ApplyLobbyModeChangeToPlayerState(APvPArenaPlayerState* PlayerState, EPvPALobbyMatchMode NewLobbyMatchMode) const
{
    if (!PlayerState)
    {
        return;
    }

    PlayerState->ResetLobbyStateForModeChange(NewLobbyMatchMode);
}

bool APvPArenaGameMode::ShouldScheduleRespawnAfterElimination(bool bHasVictimController) const
{
    const APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>();
    return ShouldScheduleRespawnAfterEliminationForMode(
        GetLobbyMatchMode(),
        bHasVictimController,
        bHasWinner,
        PvPGameState ? PvPGameState->GetRoundState() : EPvPARoundState::Playing);
}

bool APvPArenaGameMode::ShouldScheduleRespawnAfterEliminationForMode(
    EPvPALobbyMatchMode LobbyMatchMode,
    bool bHasVictimController,
    bool bRoundAlreadyHasWinner,
    EPvPARoundState CurrentRoundState) const
{
    if (!bHasVictimController || bRoundAlreadyHasWinner)
    {
        return false;
    }

    return LobbyMatchMode != EPvPALobbyMatchMode::TeamVersus && CurrentRoundState != EPvPARoundState::SuddenDeath;
}

bool APvPArenaGameMode::ShouldEnterSpectatingAfterEliminationForMode(
    EPvPALobbyMatchMode LobbyMatchMode,
    bool bHasVictimController,
    bool bRoundAlreadyHasWinner,
    EPvPARoundState CurrentRoundState) const
{
    return !ShouldScheduleRespawnAfterEliminationForMode(
            LobbyMatchMode,
            bHasVictimController,
            bRoundAlreadyHasWinner,
            CurrentRoundState)
        && bHasVictimController
        && !bRoundAlreadyHasWinner
        && (LobbyMatchMode == EPvPALobbyMatchMode::TeamVersus || CurrentRoundState == EPvPARoundState::SuddenDeath);
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

TArray<AActor*> APvPArenaGameMode::FilterPlayerStartsForPlayer(const TArray<AActor*>& CandidateStarts, const AController* Player) const
{
    const APvPArenaPlayerState* PlayerState = Player ? Cast<APvPArenaPlayerState>(Player->PlayerState) : nullptr;
    if (!PlayerState)
    {
        return CandidateStarts;
    }

    TArray<AActor*> FilteredStarts;
    FilteredStarts.Reserve(CandidateStarts.Num());

    for (AActor* CandidateStart : CandidateStarts)
    {
        const APlayerStart* PlayerStart = Cast<APlayerStart>(CandidateStart);
        if (!PlayerStart)
        {
            continue;
        }

        const FString Identifier = ResolvePlayerStartIdentifier(PlayerStart);
        bool bMatches = false;

        if (PlayerState->GetLobbyMatchMode() == EPvPALobbyMatchMode::TeamVersus)
        {
            if (PlayerState->GetLobbyTeam() == EPvPALobbyTeam::Left)
            {
                bMatches = IsBlueTeamSpawnIdentifier(Identifier);
            }
            else if (PlayerState->GetLobbyTeam() == EPvPALobbyTeam::Right)
            {
                bMatches = IsRedTeamSpawnIdentifier(Identifier);
            }
        }
        else
        {
            bMatches = IsFreeForAllSpawnIdentifier(Identifier);
        }

        if (bMatches)
        {
            FilteredStarts.Add(CandidateStart);
        }
    }

    return FilteredStarts.IsEmpty() ? CandidateStarts : FilteredStarts;
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
        PvPGameState->SetRemainingRoundTimeSeconds(ResolveRoundDurationSecondsForMode(GetLobbyMatchMode()));
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

        const TArray<AActor*> CandidateStartsForPlayer = FilterPlayerStartsForPlayer(CandidateStarts, Controller);
        if (AActor* RoundStart = ChooseRoundStartFromCandidates(CandidateStartsForPlayer, Controller, UsedRoundStartSpots))
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

    if (GetLobbyMatchMode() == EPvPALobbyMatchMode::TeamVersus)
    {
        const int32 LeftTeamAlivePlayers = CountAlivePlayersOnLobbyTeam(EPvPALobbyTeam::Left);
        const int32 RightTeamAlivePlayers = CountAlivePlayersOnLobbyTeam(EPvPALobbyTeam::Right);
        const EPvPARoundState TimeoutState = ResolveTeamVersusRoundTimeoutState(LeftTeamAlivePlayers, RightTeamAlivePlayers);
        PvPGameState->SetRoundState(TimeoutState);

        if (LeftTeamAlivePlayers > RightTeamAlivePlayers)
        {
            AwardRoundWinners(GatherPlayersOnLobbyTeam(EPvPALobbyTeam::Left), ResolveRepresentingPlayerForTeam(EPvPALobbyTeam::Left));
            return;
        }

        if (RightTeamAlivePlayers > LeftTeamAlivePlayers)
        {
            AwardRoundWinners(GatherPlayersOnLobbyTeam(EPvPALobbyTeam::Right), ResolveRepresentingPlayerForTeam(EPvPALobbyTeam::Right));
            return;
        }

        TArray<APvPArenaPlayerState*> TiedWinners = GatherPlayersOnLobbyTeam(EPvPALobbyTeam::Left);
        TiedWinners.Append(GatherPlayersOnLobbyTeam(EPvPALobbyTeam::Right));
        AwardRoundWinners(TiedWinners, nullptr);
        return;
    }

    GetWorldTimerManager().ClearTimer(RoundTimerHandle);

    const TArray<APvPArenaPlayerState*> Leaders = ResolveRoundLeadersFromScores();
    if (Leaders.Num() == 1)
    {
        bHasWinner = true;
        BeginMatchEndPhase(Leaders[0]);
        return;
    }

    EnterFreeForAllSuddenDeath(Leaders);
}

void APvPArenaGameMode::HandlePlayerEliminated(AController* VictimController, AController* KillerController)
{
    APvPArenaPlayerState* VictimState = VictimController ? Cast<APvPArenaPlayerState>(VictimController->PlayerState) : nullptr;
    APvPArenaPlayerState* KillerState = KillerController ? Cast<APvPArenaPlayerState>(KillerController->PlayerState) : nullptr;

    RegisterKill(KillerState, VictimState);

    const APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>();
    const EPvPALobbyMatchMode LobbyMatchMode = GetLobbyMatchMode();
    const EPvPARoundState CurrentRoundState = PvPGameState ? PvPGameState->GetRoundState() : EPvPARoundState::Playing;
    if (ShouldEnterSpectatingAfterEliminationForMode(
            LobbyMatchMode,
            VictimController != nullptr,
            bHasWinner,
            CurrentRoundState))
    {
        if (APvPArenaCharacter* VictimCharacter = Cast<APvPArenaCharacter>(VictimController->GetPawn()))
        {
            const float DeathAnimationDuration = VictimCharacter->GetDeathAnimationDuration();
            if (DeathAnimationDuration > 0.0f)
            {
                FTimerHandle SpectatorTimerHandle;
                TWeakObjectPtr<AController> VictimControllerWeak = VictimController;
                GetWorldTimerManager().SetTimer(
                    SpectatorTimerHandle,
                    FTimerDelegate::CreateLambda([this, VictimControllerWeak]()
                    {
                        if (!VictimControllerWeak.IsValid())
                        {
                            return;
                        }

                        if (APawn* ExistingPawn = VictimControllerWeak->GetPawn())
                        {
                            ExistingPawn->Destroy();
                        }

                        SetControllerSpectating(VictimControllerWeak.Get());
                    }),
                    DeathAnimationDuration,
                    false);
            }
            else
            {
                VictimCharacter->Destroy();
                SetControllerSpectating(VictimController);
            }
        }
        else
        {
            SetControllerSpectating(VictimController);
        }
        return;
    }

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

        const APvPArenaGameState* CurrentGameState = GetGameState<APvPArenaGameState>();
        if (!ShouldScheduleRespawnAfterEliminationForMode(
                GetLobbyMatchMode(),
                true,
                bHasWinner,
                CurrentGameState ? CurrentGameState->GetRoundState() : EPvPARoundState::Playing))
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
        const EPvPALobbyMatchMode LobbyMatchMode = GetLobbyMatchMode();
        PvPGameState->SetScoreLimit(ResolveScoreLimitForMode(LobbyMatchMode));
        PvPGameState->SetRoundWinsToWin(ResolveRoundWinsToWinForMode(LobbyMatchMode));
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
        const EPvPALobbyMatchMode LobbyMatchMode = GetLobbyMatchMode();
        PvPGameState->SetScoreLimit(ResolveScoreLimitForMode(LobbyMatchMode));
        PvPGameState->SetRoundWinsToWin(ResolveRoundWinsToWinForMode(LobbyMatchMode));
        PvPGameState->SetMatchPhase(EPvPAMatchPhase::Playing);
        PvPGameState->SetRoundState(EPvPARoundState::Playing);
        PvPGameState->SetRemainingRoundTimeSeconds(ResolveRoundDurationSecondsForMode(LobbyMatchMode));
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

void APvPArenaGameMode::AwardRoundWinners(const TArray<APvPArenaPlayerState*>& RoundWinners, APvPArenaPlayerState* DisplayWinner)
{
    if (RoundWinners.IsEmpty())
    {
        if (APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>())
        {
            PvPGameState->SetRoundWinner(nullptr);
        }
        BeginRoundEndPhase();
        return;
    }

    for (APvPArenaPlayerState* Winner : RoundWinners)
    {
        if (Winner)
        {
            Winner->AddRoundWin();
        }
    }

    if (APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>())
    {
        PvPGameState->SetRoundWinner(DisplayWinner);
    }

    int32 HighestRoundWins = 0;
    const bool bHasTieAtHighestRoundWins = HasTieAtHighestRoundWins(HighestRoundWins);
    if (ShouldEndMatchOnRoundWinState(HighestRoundWins, bHasTieAtHighestRoundWins))
    {
        BeginMatchEndPhase(ResolveUniqueMatchWinner());
        return;
    }

    BeginRoundEndPhase();
}

APvPArenaPlayerState* APvPArenaGameMode::ResolveRoundWinnerFromScores() const
{
    const TArray<APvPArenaPlayerState*> Leaders = ResolveRoundLeadersFromScores();
    return Leaders.Num() == 1 ? Leaders[0] : nullptr;
}

TArray<APvPArenaPlayerState*> APvPArenaGameMode::ResolveRoundLeadersFromScores() const
{
    TArray<APvPArenaPlayerState*> Leaders;
    const AGameStateBase* BaseGameState = GameState;
    if (!BaseGameState)
    {
        return Leaders;
    }

    int32 BestKills = TNumericLimits<int32>::Min();
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
            Leaders.Reset();
            Leaders.Add(PvPState);
        }
        else if (RoundKills == BestKills)
        {
            Leaders.Add(PvPState);
        }
    }

    return Leaders;
}

TArray<APvPArenaPlayerState*> APvPArenaGameMode::GatherPlayersOnLobbyTeam(EPvPALobbyTeam LobbyTeam) const
{
    TArray<APvPArenaPlayerState*> TeamPlayers;
    const AGameStateBase* BaseGameState = GameState;
    if (!BaseGameState)
    {
        return TeamPlayers;
    }

    for (APlayerState* PlayerState : BaseGameState->PlayerArray)
    {
        APvPArenaPlayerState* PvPPlayerState = Cast<APvPArenaPlayerState>(PlayerState);
        if (PvPPlayerState && PvPPlayerState->GetLobbyTeam() == LobbyTeam)
        {
            TeamPlayers.Add(PvPPlayerState);
        }
    }

    return TeamPlayers;
}

int32 APvPArenaGameMode::CountAlivePlayersOnLobbyTeam(EPvPALobbyTeam LobbyTeam) const
{
    if (!GetWorld())
    {
        return 0;
    }

    int32 AlivePlayers = 0;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        const AController* Controller = It->Get();
        const APvPArenaPlayerState* PlayerState = Controller ? Cast<APvPArenaPlayerState>(Controller->PlayerState) : nullptr;
        const APvPArenaCharacter* Character = Controller ? Cast<APvPArenaCharacter>(Controller->GetPawn()) : nullptr;
        if (PlayerState && Character && !Character->IsDead() && PlayerState->GetLobbyTeam() == LobbyTeam)
        {
            ++AlivePlayers;
        }
    }

    return AlivePlayers;
}

int32 APvPArenaGameMode::GetTeamRoundWins(EPvPALobbyTeam LobbyTeam) const
{
    int32 TeamRoundWins = 0;
    const AGameStateBase* BaseGameState = GameState;
    if (!BaseGameState)
    {
        return 0;
    }

    for (APlayerState* PlayerState : BaseGameState->PlayerArray)
    {
        const APvPArenaPlayerState* PvPPlayerState = Cast<APvPArenaPlayerState>(PlayerState);
        if (PvPPlayerState && PvPPlayerState->GetLobbyTeam() == LobbyTeam)
        {
            TeamRoundWins = FMath::Max(TeamRoundWins, PvPPlayerState->GetRoundWins());
        }
    }

    return TeamRoundWins;
}

APvPArenaPlayerState* APvPArenaGameMode::ResolveRepresentingPlayerForTeam(EPvPALobbyTeam LobbyTeam) const
{
    TArray<APvPArenaPlayerState*> TeamPlayers = GatherPlayersOnLobbyTeam(LobbyTeam);
    return TeamPlayers.IsEmpty() ? nullptr : TeamPlayers[0];
}

bool APvPArenaGameMode::HasTieAtHighestRoundWins(int32& OutHighestRoundWins) const
{
    OutHighestRoundWins = 0;
    if (GetLobbyMatchMode() == EPvPALobbyMatchMode::TeamVersus)
    {
        const int32 LeftTeamRoundWins = GetTeamRoundWins(EPvPALobbyTeam::Left);
        const int32 RightTeamRoundWins = GetTeamRoundWins(EPvPALobbyTeam::Right);
        OutHighestRoundWins = FMath::Max(LeftTeamRoundWins, RightTeamRoundWins);
        return LeftTeamRoundWins == RightTeamRoundWins;
    }

    bool bFoundAnyPlayer = false;
    bool bHasTie = false;
    const AGameStateBase* BaseGameState = GameState;
    if (!BaseGameState)
    {
        return false;
    }

    for (APlayerState* PlayerState : BaseGameState->PlayerArray)
    {
        const APvPArenaPlayerState* PvPState = Cast<APvPArenaPlayerState>(PlayerState);
        if (!PvPState)
        {
            continue;
        }

        const int32 RoundWins = PvPState->GetRoundWins();
        if (!bFoundAnyPlayer || RoundWins > OutHighestRoundWins)
        {
            OutHighestRoundWins = RoundWins;
            bFoundAnyPlayer = true;
            bHasTie = false;
        }
        else if (RoundWins == OutHighestRoundWins)
        {
            bHasTie = true;
        }
    }

    return bHasTie;
}

APvPArenaPlayerState* APvPArenaGameMode::ResolveUniqueMatchWinner() const
{
    if (GetLobbyMatchMode() == EPvPALobbyMatchMode::TeamVersus)
    {
        const int32 LeftTeamRoundWins = GetTeamRoundWins(EPvPALobbyTeam::Left);
        const int32 RightTeamRoundWins = GetTeamRoundWins(EPvPALobbyTeam::Right);
        if (LeftTeamRoundWins == RightTeamRoundWins || FMath::Max(LeftTeamRoundWins, RightTeamRoundWins) < ResolveRoundWinsToWinForMode(EPvPALobbyMatchMode::TeamVersus))
        {
            return nullptr;
        }

        return ResolveRepresentingPlayerForTeam(
            LeftTeamRoundWins > RightTeamRoundWins ? EPvPALobbyTeam::Left : EPvPALobbyTeam::Right);
    }

    const AGameStateBase* BaseGameState = GameState;
    if (!BaseGameState)
    {
        return nullptr;
    }

    APvPArenaPlayerState* MatchWinner = nullptr;
    int32 HighestRoundWins = 0;
    bool bFoundAnyPlayer = false;
    bool bHasTie = false;
    for (APlayerState* PlayerState : BaseGameState->PlayerArray)
    {
        APvPArenaPlayerState* PvPState = Cast<APvPArenaPlayerState>(PlayerState);
        if (!PvPState)
        {
            continue;
        }

        if (!bFoundAnyPlayer || PvPState->GetRoundWins() > HighestRoundWins)
        {
            HighestRoundWins = PvPState->GetRoundWins();
            MatchWinner = PvPState;
            bFoundAnyPlayer = true;
            bHasTie = false;
        }
        else if (PvPState->GetRoundWins() == HighestRoundWins)
        {
            bHasTie = true;
        }
    }

    return bHasTie || HighestRoundWins < ResolveRoundWinsToWinForMode(EPvPALobbyMatchMode::TeamVersus) ? nullptr : MatchWinner;
}

int32 APvPArenaGameMode::ResolveScoreLimitForMode(EPvPALobbyMatchMode LobbyMatchMode) const
{
    return LobbyMatchMode == EPvPALobbyMatchMode::FreeForAll ? ScoreLimit : ScoreLimit;
}

int32 APvPArenaGameMode::ResolveRoundDurationSecondsForMode(EPvPALobbyMatchMode LobbyMatchMode) const
{
    return LobbyMatchMode == EPvPALobbyMatchMode::FreeForAll
        ? RoundDurationSeconds
        : TeamVersusRoundDurationSecondsDefault;
}

int32 APvPArenaGameMode::ResolveRoundWinsToWinForMode(EPvPALobbyMatchMode LobbyMatchMode) const
{
    return LobbyMatchMode == EPvPALobbyMatchMode::TeamVersus ? IterationRoundWinsToWinDefault : 1;
}

void APvPArenaGameMode::EnterFreeForAllSuddenDeath(const TArray<APvPArenaPlayerState*>& TiedLeaders)
{
    APvPArenaGameState* PvPGameState = GetGameState<APvPArenaGameState>();
    if (!PvPGameState || TiedLeaders.Num() <= 1)
    {
        return;
    }

    bHasWinner = false;
    PvPGameState->SetRoundState(EPvPARoundState::SuddenDeath);
    PvPGameState->SetRemainingRoundTimeSeconds(0);
    PvPGameState->SetRemainingRoundEndTimeSeconds(0);
    PvPGameState->SetRoundWinner(nullptr);
    ResetControllersForFreeForAllSuddenDeath(TiedLeaders);
}

void APvPArenaGameMode::ResetControllersForFreeForAllSuddenDeath(const TArray<APvPArenaPlayerState*>& TiedLeaders)
{
    if (!GetWorld())
    {
        return;
    }

    TSet<TObjectKey<APvPArenaPlayerState>> LeaderKeys;
    for (APvPArenaPlayerState* Leader : TiedLeaders)
    {
        if (Leader)
        {
            LeaderKeys.Add(Leader);
        }
    }

    TSet<TObjectKey<AActor>> UsedRoundStartSpots;
    TArray<AActor*> CandidateStarts;
    UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), CandidateStarts);

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        AController* Controller = It->Get();
        APvPArenaPlayerState* PlayerState = Controller ? Cast<APvPArenaPlayerState>(Controller->PlayerState) : nullptr;
        if (!Controller || !PlayerState || !LeaderKeys.Contains(PlayerState))
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
    }

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        AController* Controller = It->Get();
        APvPArenaPlayerState* PlayerState = Controller ? Cast<APvPArenaPlayerState>(Controller->PlayerState) : nullptr;
        if (!Controller || !PlayerState || LeaderKeys.Contains(PlayerState))
        {
            continue;
        }

        if (APawn* ExistingPawn = Controller->GetPawn())
        {
            ExistingPawn->Destroy();
        }

        SetControllerSpectating(Controller);
    }
}

void APvPArenaGameMode::SetControllerSpectating(AController* Controller) const
{
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        PlayerController->StartSpectatingOnly();
        PlayerController->ChangeState(NAME_Spectating);
        PlayerController->ClientGotoState(NAME_Spectating);
        if (APvPArenaPlayerController* PvPPlayerController = Cast<APvPArenaPlayerController>(PlayerController))
        {
            PvPPlayerController->ClientEnterFreeSpectatorMode();
        }
    }
}

EPvPALobbyMatchMode APvPArenaGameMode::GetLobbyMatchMode() const
{
    const AGameStateBase* BaseGameState = GameState;
    if (!BaseGameState)
    {
        return EPvPALobbyMatchMode::FreeForAll;
    }

    for (APlayerState* PlayerState : BaseGameState->PlayerArray)
    {
        const APvPArenaPlayerState* PvPPlayerState = Cast<APvPArenaPlayerState>(PlayerState);
        if (PvPPlayerState)
        {
            return PvPPlayerState->GetLobbyMatchMode();
        }
    }

    return EPvPALobbyMatchMode::FreeForAll;
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

FString APvPArenaGameMode::ResolveUniqueDefaultDisplayNickname(const APvPArenaPlayerState* ExcludedPlayerState) const
{
    const AGameStateBase* BaseGameState = GameState;
    int32 CandidateIndex = 1;

    while (true)
    {
        const FString CandidateNickname = BuildDefaultDisplayNickname(CandidateIndex++);
        bool bNameAlreadyInUse = false;

        if (BaseGameState)
        {
            for (APlayerState* PlayerState : BaseGameState->PlayerArray)
            {
                const APvPArenaPlayerState* PvPPlayerState = Cast<APvPArenaPlayerState>(PlayerState);
                if (!PvPPlayerState || PvPPlayerState == ExcludedPlayerState)
                {
                    continue;
                }

                if (PvPPlayerState->GetDisplayNickname().Equals(CandidateNickname, ESearchCase::CaseSensitive))
                {
                    bNameAlreadyInUse = true;
                    break;
                }
            }
        }

        if (!bNameAlreadyInUse)
        {
            return CandidateNickname;
        }
    }
}

int32 APvPArenaGameMode::CountPlayersOnLobbyTeam(EPvPALobbyTeam LobbyTeam) const
{
    const AGameStateBase* BaseGameState = GameState;
    if (!BaseGameState)
    {
        return 0;
    }

    int32 TeamPlayers = 0;
    for (APlayerState* PlayerState : BaseGameState->PlayerArray)
    {
        const APvPArenaPlayerState* PvPPlayerState = Cast<APvPArenaPlayerState>(PlayerState);
        if (PvPPlayerState && PvPPlayerState->GetLobbyTeam() == LobbyTeam)
        {
            ++TeamPlayers;
        }
    }

    return TeamPlayers;
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
