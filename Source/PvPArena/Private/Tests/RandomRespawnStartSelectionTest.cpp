#include "Misc/AutomationTest.h"
#include "Game/PvPArenaGameMode.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRandomRespawnStartSelectionTest,
    "PvPArena.Match.RandomRespawnStartSelection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRandomRespawnStartSelectionTest::RunTest(const FString& Parameters)
{
    APvPArenaGameMode* GameMode = NewObject<APvPArenaGameMode>();
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

    return true;
}
