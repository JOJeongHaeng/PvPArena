#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMatchPhaseReplicationTest,
    "PvPArena.Game.MatchPhaseReplication",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPhaseReplicationTest::RunTest(const FString& Parameters)
{
    UClass* GameStateClass = FindObject<UClass>(nullptr, TEXT("/Script/PvPArena.PvPArenaGameState"));
    TestNotNull(TEXT("PvPArenaGameState class should exist"), GameStateClass);

    if (!GameStateClass)
    {
        return false;
    }

    const FProperty* MatchPhaseProp = FindFProperty<FProperty>(GameStateClass, TEXT("MatchPhase"));
    const FProperty* RemainingSecondsProp = FindFProperty<FProperty>(GameStateClass, TEXT("RemainingSeconds"));
    const FProperty* TeamAScoreProp = FindFProperty<FProperty>(GameStateClass, TEXT("TeamAScore"));
    const FProperty* TeamBScoreProp = FindFProperty<FProperty>(GameStateClass, TEXT("TeamBScore"));

    TestNotNull(TEXT("MatchPhase property should exist"), MatchPhaseProp);
    TestNotNull(TEXT("RemainingSeconds property should exist"), RemainingSecondsProp);
    TestNotNull(TEXT("TeamAScore property should exist"), TeamAScoreProp);
    TestNotNull(TEXT("TeamBScore property should exist"), TeamBScoreProp);

    if (MatchPhaseProp)
    {
        TestTrue(TEXT("MatchPhase should be replicated"), MatchPhaseProp->HasAnyPropertyFlags(CPF_Net));
    }
    if (RemainingSecondsProp)
    {
        TestTrue(TEXT("RemainingSeconds should be replicated"), RemainingSecondsProp->HasAnyPropertyFlags(CPF_Net));
    }
    if (TeamAScoreProp)
    {
        TestTrue(TEXT("TeamAScore should be replicated"), TeamAScoreProp->HasAnyPropertyFlags(CPF_Net));
    }
    if (TeamBScoreProp)
    {
        TestTrue(TEXT("TeamBScore should be replicated"), TeamBScoreProp->HasAnyPropertyFlags(CPF_Net));
    }

    return true;
}
