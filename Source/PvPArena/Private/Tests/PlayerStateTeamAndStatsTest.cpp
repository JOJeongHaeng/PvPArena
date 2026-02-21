#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPlayerStateTeamAndStatsTest,
    "PvPArena.Player.TeamAndStats",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerStateTeamAndStatsTest::RunTest(const FString& Parameters)
{
    UClass* PlayerStateClass = FindObject<UClass>(nullptr, TEXT("/Script/PvPArena.PvPArenaPlayerState"));
    TestNotNull(TEXT("PvPArenaPlayerState class should exist"), PlayerStateClass);

    if (!PlayerStateClass)
    {
        return false;
    }

    const FProperty* TeamIdProp = FindFProperty<FProperty>(PlayerStateClass, TEXT("TeamId"));
    const FProperty* KillsProp = FindFProperty<FProperty>(PlayerStateClass, TEXT("Kills"));
    const FProperty* DeathsProp = FindFProperty<FProperty>(PlayerStateClass, TEXT("Deaths"));
    const FProperty* IsDeadProp = FindFProperty<FProperty>(PlayerStateClass, TEXT("bIsDead"));
    const FProperty* RespawnRemainingProp = FindFProperty<FProperty>(PlayerStateClass, TEXT("RespawnRemainingSeconds"));

    TestNotNull(TEXT("TeamId property should exist"), TeamIdProp);
    TestNotNull(TEXT("Kills property should exist"), KillsProp);
    TestNotNull(TEXT("Deaths property should exist"), DeathsProp);
    TestNotNull(TEXT("bIsDead property should exist"), IsDeadProp);
    TestNotNull(TEXT("RespawnRemainingSeconds property should exist"), RespawnRemainingProp);

    if (TeamIdProp)
    {
        TestTrue(TEXT("TeamId should be replicated"), TeamIdProp->HasAnyPropertyFlags(CPF_Net));
    }
    if (KillsProp)
    {
        TestTrue(TEXT("Kills should be replicated"), KillsProp->HasAnyPropertyFlags(CPF_Net));
    }
    if (DeathsProp)
    {
        TestTrue(TEXT("Deaths should be replicated"), DeathsProp->HasAnyPropertyFlags(CPF_Net));
    }
    if (IsDeadProp)
    {
        TestTrue(TEXT("bIsDead should be replicated"), IsDeadProp->HasAnyPropertyFlags(CPF_Net));
    }
    if (RespawnRemainingProp)
    {
        TestTrue(TEXT("RespawnRemainingSeconds should be replicated"), RespawnRemainingProp->HasAnyPropertyFlags(CPF_Net));
    }

    UFunction* SetTeamIdFunc = PlayerStateClass->FindFunctionByName(TEXT("SetTeamId"));
    TestNotNull(TEXT("SetTeamId function should exist for server-side team assignment"), SetTeamIdFunc);

    return true;
}
