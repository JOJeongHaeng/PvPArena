#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "UObject/Class.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FEndConditionAndLoopTest,
    "PvPArena.Game.EndConditionAndLoop",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEndConditionAndLoopTest::RunTest(const FString& Parameters)
{
    UClass* GameStateClass = FindObject<UClass>(nullptr, TEXT("/Script/PvPArena.PvPArenaGameState"));
    UClass* GameModeClass = FindObject<UClass>(nullptr, TEXT("/Script/PvPArena.PvPArenaGameMode"));

    TestNotNull(TEXT("PvPArenaGameState class should exist"), GameStateClass);
    TestNotNull(TEXT("PvPArenaGameMode class should exist"), GameModeClass);

    if (!GameStateClass || !GameModeClass)
    {
        return false;
    }

    UFunction* GetMatchEndReasonFunc = GameStateClass->FindFunctionByName(TEXT("GetMatchEndReason"));
    UFunction* EvaluateEndConditionsFunc = GameModeClass->FindFunctionByName(TEXT("EvaluateEndConditions"));

    TestNotNull(TEXT("GetMatchEndReason should exist"), GetMatchEndReasonFunc);
    TestNotNull(TEXT("EvaluateEndConditions should exist"), EvaluateEndConditionsFunc);

    return true;
}
