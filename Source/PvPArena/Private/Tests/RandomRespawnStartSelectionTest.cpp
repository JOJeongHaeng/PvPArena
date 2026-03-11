#include "Misc/AutomationTest.h"
#include "Game/PvPArenaGameMode.h"
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
    TestNotNull(TEXT("StartA should be created"), StartA);
    TestNotNull(TEXT("StartB should be created"), StartB);

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

    return true;
}
