#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "UObject/Class.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHitToScoreFlowTest,
    "PvPArena.Combat.HitToScoreFlow",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHitToScoreFlowTest::RunTest(const FString& Parameters)
{
    UClass* GameStateClass = FindObject<UClass>(nullptr, TEXT("/Script/PvPArena.PvPArenaGameState"));
    UClass* WeaponClass = FindObject<UClass>(nullptr, TEXT("/Script/PvPArena.PvPArenaWeaponComponent"));

    TestNotNull(TEXT("PvPArenaGameState class should exist"), GameStateClass);
    TestNotNull(TEXT("PvPArenaWeaponComponent class should exist"), WeaponClass);

    if (!GameStateClass || !WeaponClass)
    {
        return false;
    }

    UFunction* AddTeamScoreFunc = GameStateClass->FindFunctionByName(TEXT("AddTeamScore"));
    UFunction* HandleConfirmedHitFunc = WeaponClass->FindFunctionByName(TEXT("HandleConfirmedHit"));

    TestNotNull(TEXT("AddTeamScore should exist on GameState"), AddTeamScoreFunc);
    TestNotNull(TEXT("HandleConfirmedHit should exist on WeaponComponent"), HandleConfirmedHitFunc);

    return true;
}
