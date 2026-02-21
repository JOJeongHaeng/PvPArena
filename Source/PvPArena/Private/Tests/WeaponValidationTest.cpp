#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FWeaponValidationTest,
    "PvPArena.Combat.WeaponValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWeaponValidationTest::RunTest(const FString& Parameters)
{
    UClass* WeaponClass = FindObject<UClass>(nullptr, TEXT("/Script/PvPArena.PvPArenaWeaponComponent"));
    TestNotNull(TEXT("PvPArenaWeaponComponent class should exist"), WeaponClass);

    if (!WeaponClass)
    {
        return false;
    }

    const FProperty* CurrentAmmoProp = FindFProperty<FProperty>(WeaponClass, TEXT("CurrentAmmo"));
    const FProperty* bIsReloadingProp = FindFProperty<FProperty>(WeaponClass, TEXT("bIsReloading"));

    TestNotNull(TEXT("CurrentAmmo property should exist"), CurrentAmmoProp);
    TestNotNull(TEXT("bIsReloading property should exist"), bIsReloadingProp);

    if (CurrentAmmoProp)
    {
        TestTrue(TEXT("CurrentAmmo should be replicated"), CurrentAmmoProp->HasAnyPropertyFlags(CPF_Net));
    }

    UFunction* ServerTryFireFunc = WeaponClass->FindFunctionByName(TEXT("ServerTryFire"));
    UFunction* ServerReloadFunc = WeaponClass->FindFunctionByName(TEXT("ServerReload"));

    TestNotNull(TEXT("ServerTryFire RPC should exist"), ServerTryFireFunc);
    TestNotNull(TEXT("ServerReload RPC should exist"), ServerReloadFunc);

    return true;
}
