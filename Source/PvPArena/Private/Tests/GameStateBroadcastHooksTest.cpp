#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "UObject/Class.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGameStateBroadcastHooksTest,
    "PvPArena.UI.GameStateBroadcastHooks",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGameStateBroadcastHooksTest::RunTest(const FString& Parameters)
{
    UClass* GameStateClass = FindObject<UClass>(nullptr, TEXT("/Script/PvPArena.PvPArenaGameState"));
    TestNotNull(TEXT("PvPArenaGameState class should exist"), GameStateClass);

    if (!GameStateClass)
    {
        return false;
    }

    UFunction* OnRepMatchPhase = GameStateClass->FindFunctionByName(TEXT("OnRep_MatchPhase"));
    UFunction* OnRepRemainingSeconds = GameStateClass->FindFunctionByName(TEXT("OnRep_RemainingSeconds"));
    UFunction* OnRepTeamAScore = GameStateClass->FindFunctionByName(TEXT("OnRep_TeamAScore"));
    UFunction* OnRepTeamBScore = GameStateClass->FindFunctionByName(TEXT("OnRep_TeamBScore"));
    FProperty* OnMatchPhaseChanged = GameStateClass->FindPropertyByName(TEXT("OnMatchPhaseChanged"));
    FProperty* OnRemainingSecondsChanged = GameStateClass->FindPropertyByName(TEXT("OnRemainingSecondsChanged"));
    FProperty* OnTeamScoreChanged = GameStateClass->FindPropertyByName(TEXT("OnTeamScoreChanged"));

    TestNotNull(TEXT("OnRep_MatchPhase should exist"), OnRepMatchPhase);
    TestNotNull(TEXT("OnRep_RemainingSeconds should exist"), OnRepRemainingSeconds);
    TestNotNull(TEXT("OnRep_TeamAScore should exist"), OnRepTeamAScore);
    TestNotNull(TEXT("OnRep_TeamBScore should exist"), OnRepTeamBScore);
    TestNotNull(TEXT("OnMatchPhaseChanged delegate should exist"), OnMatchPhaseChanged);
    TestNotNull(TEXT("OnRemainingSecondsChanged delegate should exist"), OnRemainingSecondsChanged);
    TestNotNull(TEXT("OnTeamScoreChanged delegate should exist"), OnTeamScoreChanged);

    return true;
}
