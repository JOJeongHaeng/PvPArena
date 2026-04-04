#include "Misc/AutomationTest.h"
#include "Game/PvPArenaGameMode.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"

namespace
{
class FTestPvPArenaGameMode : public APvPArenaGameMode
{
public:
    bool ShouldReuseCachedStartSpot(AController* Player)
    {
        return ShouldSpawnAtStartSpot(Player);
    }

    AActor* ChooseRoundStartForPlayer(const TArray<AActor*>& CandidateStarts, AController* Player, const TSet<TObjectKey<AActor>>& UsedStarts)
    {
        return ChooseRoundStartFromCandidates(CandidateStarts, Player, UsedStarts);
    }

    TArray<AActor*> FilterStartsForPlayer(const TArray<AActor*>& CandidateStarts, const AController* Player) const
    {
        return FilterPlayerStartsForPlayer(CandidateStarts, Player);
    }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRandomRespawnStartSelectionTest,
    "PvPArena.Match.RandomRespawnStartSelection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRandomRespawnStartSelectionTest::RunTest(const FString& Parameters)
{
    FTestPvPArenaGameMode* GameMode = NewObject<FTestPvPArenaGameMode>();
    TestNotNull(TEXT("GameMode should be created"), GameMode);

    if (!GameMode)
    {
        return false;
    }

    APlayerStart* StartA = NewObject<APlayerStart>();
    APlayerStart* StartB = NewObject<APlayerStart>();
    APlayerController* PlayerOne = NewObject<APlayerController>();
    APlayerController* PlayerTwo = NewObject<APlayerController>();
    TestNotNull(TEXT("StartA should be created"), StartA);
    TestNotNull(TEXT("StartB should be created"), StartB);
    TestNotNull(TEXT("PlayerOne should be created"), PlayerOne);
    TestNotNull(TEXT("PlayerTwo should be created"), PlayerTwo);

    TArray<AActor*> MultipleCandidates;
    MultipleCandidates.Add(StartA);
    MultipleCandidates.Add(StartB);

    TestEqual(
        TEXT("When alternatives exist, the previous spawn should be avoided"),
        GameMode->ChooseRespawnStartFromCandidates(MultipleCandidates, StartA),
        static_cast<AActor*>(StartB));

    TArray<AActor*> SingleCandidate;
    SingleCandidate.Add(StartA);

    TestEqual(
        TEXT("When only one candidate exists, reusing it is allowed"),
        GameMode->ChooseRespawnStartFromCandidates(SingleCandidate, StartA),
        static_cast<AActor*>(StartA));

    const AActor* PlayerOneRoundOneStart = GameMode->ChooseRespawnStartForPlayer(MultipleCandidates, PlayerOne);
    const AActor* PlayerTwoRoundOneStart = GameMode->ChooseRespawnStartForPlayer(MultipleCandidates, PlayerTwo);
    const AActor* PlayerOneRoundTwoStart = GameMode->ChooseRespawnStartForPlayer(MultipleCandidates, PlayerOne);
    const AActor* PlayerTwoRoundTwoStart = GameMode->ChooseRespawnStartForPlayer(MultipleCandidates, PlayerTwo);

    TestNotEqual(
        TEXT("Player one should avoid reusing its own previous start when another start exists"),
        PlayerOneRoundTwoStart,
        PlayerOneRoundOneStart);

    TestNotEqual(
        TEXT("Player two should avoid reusing its own previous start when another start exists"),
        PlayerTwoRoundTwoStart,
        PlayerTwoRoundOneStart);

    PlayerOne->StartSpot = StartA;
    TestFalse(
        TEXT("Respawn flow should not reuse the controller's cached StartSpot"),
        GameMode->ShouldReuseCachedStartSpot(PlayerOne));

    TSet<TObjectKey<AActor>> UsedRoundStartSpots;
    const AActor* RoundStartForPlayerOne = GameMode->ChooseRoundStartForPlayer(MultipleCandidates, PlayerOne, UsedRoundStartSpots);
    if (RoundStartForPlayerOne)
    {
        UsedRoundStartSpots.Add(RoundStartForPlayerOne);
    }

    const AActor* RoundStartForPlayerTwo = GameMode->ChooseRoundStartForPlayer(MultipleCandidates, PlayerTwo, UsedRoundStartSpots);
    TestNotEqual(
        TEXT("Round reset should give different starts to each player when alternatives exist"),
        RoundStartForPlayerTwo,
        RoundStartForPlayerOne);

    APlayerStart* LeftTeamStart = NewObject<APlayerStart>(GetTransientPackage(), TEXT("LeftTeamStart"));
    APlayerStart* RightTeamStart = NewObject<APlayerStart>(GetTransientPackage(), TEXT("RightTeamStart"));
    APlayerStart* FreeForAllStart = NewObject<APlayerStart>(GetTransientPackage(), TEXT("FreeForAllStart"));
    TestNotNull(TEXT("LeftTeamStart should be created"), LeftTeamStart);
    TestNotNull(TEXT("RightTeamStart should be created"), RightTeamStart);
    TestNotNull(TEXT("FreeForAllStart should be created"), FreeForAllStart);

    LeftTeamStart->PlayerStartTag = TEXT("TeamLeft");
    RightTeamStart->PlayerStartTag = TEXT("TeamRight");
    FreeForAllStart->PlayerStartTag = TEXT("FreeForAll");

    APvPArenaPlayerState* LeftTeamPlayerState = NewObject<APvPArenaPlayerState>(PlayerOne);
    APvPArenaPlayerState* FreeForAllPlayerState = NewObject<APvPArenaPlayerState>(PlayerTwo);
    TestNotNull(TEXT("LeftTeam player state should be created"), LeftTeamPlayerState);
    TestNotNull(TEXT("Free-for-all player state should be created"), FreeForAllPlayerState);

    if (!LeftTeamPlayerState || !FreeForAllPlayerState)
    {
        return false;
    }

    LeftTeamPlayerState->SetLobbyMatchMode(EPvPALobbyMatchMode::TeamVersus);
    LeftTeamPlayerState->SetLobbyTeam(EPvPALobbyTeam::Left);
    PlayerOne->PlayerState = LeftTeamPlayerState;

    FreeForAllPlayerState->SetLobbyMatchMode(EPvPALobbyMatchMode::FreeForAll);
    FreeForAllPlayerState->SetLobbyTeam(EPvPALobbyTeam::None);
    PlayerTwo->PlayerState = FreeForAllPlayerState;

    TArray<AActor*> TaggedCandidates;
    TaggedCandidates.Add(LeftTeamStart);
    TaggedCandidates.Add(RightTeamStart);
    TaggedCandidates.Add(FreeForAllStart);

    const TArray<AActor*> LeftTeamCandidates = GameMode->FilterStartsForPlayer(TaggedCandidates, PlayerOne);
    TestEqual(TEXT("Left team players should only consider left-team starts"), LeftTeamCandidates.Num(), 1);
    TestEqual(TEXT("Left team players should resolve the left-team start"), LeftTeamCandidates[0], static_cast<AActor*>(LeftTeamStart));

    const TArray<AActor*> FreeForAllCandidates = GameMode->FilterStartsForPlayer(TaggedCandidates, PlayerTwo);
    TestEqual(TEXT("Free-for-all players should only consider free-for-all starts"), FreeForAllCandidates.Num(), 1);
    TestEqual(TEXT("Free-for-all players should resolve the free-for-all start"), FreeForAllCandidates[0], static_cast<AActor*>(FreeForAllStart));

    return true;
}
