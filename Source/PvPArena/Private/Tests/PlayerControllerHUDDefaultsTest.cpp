#include "Misc/AutomationTest.h"
#include "Game/PvPArenaPlayerController.h"
#include "UI/PvPArenaHUDWidget.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPlayerControllerHUDDefaultsTest,
    "PvPArena.UI.PlayerControllerHUDDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerControllerHUDDefaultsTest::RunTest(const FString& Parameters)
{
    const APvPArenaPlayerController* PlayerController = GetDefault<APvPArenaPlayerController>();
    TestNotNull(TEXT("PlayerController CDO should exist"), PlayerController);

    const FClassProperty* HUDClassProperty = FindFProperty<FClassProperty>(APvPArenaPlayerController::StaticClass(), TEXT("HUDWidgetClass"));
    TestNotNull(TEXT("HUDWidgetClass property should exist"), HUDClassProperty);

    const UClass* HUDClassValue = (PlayerController && HUDClassProperty)
        ? Cast<UClass>(HUDClassProperty->GetPropertyValue_InContainer(PlayerController))
        : nullptr;

    TestTrue(TEXT("PlayerController should default to UPvPArenaHUDWidget"),
        HUDClassValue == UPvPArenaHUDWidget::StaticClass());
    return true;
}
