#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterLifecycleTest,
    "PvPArena.Player.CharacterLifecycle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterLifecycleTest::RunTest(const FString& Parameters)
{
    UClass* CharacterClass = FindObject<UClass>(nullptr, TEXT("/Script/PvPArena.PvPArenaCharacter"));
    TestNotNull(TEXT("PvPArenaCharacter class should exist"), CharacterClass);

    if (!CharacterClass)
    {
        return false;
    }

    const FProperty* CurrentHealthProp = FindFProperty<FProperty>(CharacterClass, TEXT("CurrentHealth"));
    const FProperty* IsDeadProp = FindFProperty<FProperty>(CharacterClass, TEXT("bIsDead"));

    TestNotNull(TEXT("CurrentHealth should exist"), CurrentHealthProp);
    TestNotNull(TEXT("bIsDead should exist"), IsDeadProp);

    if (CurrentHealthProp)
    {
        TestTrue(TEXT("CurrentHealth should be replicated"), CurrentHealthProp->HasAnyPropertyFlags(CPF_Net));
    }

    UFunction* ApplyDamageServerFunc = CharacterClass->FindFunctionByName(TEXT("ApplyDamageServer"));
    UFunction* CanRespawnAtTimeFunc = CharacterClass->FindFunctionByName(TEXT("CanRespawnAtTime"));

    TestNotNull(TEXT("ApplyDamageServer function should exist"), ApplyDamageServerFunc);
    TestNotNull(TEXT("CanRespawnAtTime function should exist"), CanRespawnAtTimeFunc);

    UClass* PlayerControllerClass = FindObject<UClass>(nullptr, TEXT("/Script/PvPArena.PvPArenaPlayerController"));
    TestNotNull(TEXT("PvPArenaPlayerController class should exist"), PlayerControllerClass);

    if (PlayerControllerClass)
    {
        UFunction* ServerRequestRespawnFunc = PlayerControllerClass->FindFunctionByName(TEXT("ServerRequestRespawn"));
        TestNotNull(TEXT("ServerRequestRespawn RPC should exist"), ServerRequestRespawnFunc);
    }

    return true;
}
