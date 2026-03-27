#include "Misc/AutomationTest.h"
#include "UI/PvPArenaHUDWidget.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHUDWidgetOnlineControlsTest,
    "PvPArena.UI.HUDWidgetOnlineControls",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDWidgetOnlineControlsTest::RunTest(const FString& Parameters)
{
    const UClass* WidgetClass = UPvPArenaHUDWidget::StaticClass();

    const FObjectProperty* ConnectionStatusTextProperty =
        FindFProperty<FObjectProperty>(WidgetClass, TEXT("ConnectionStatusText"));
    TestNotNull(TEXT("HUD widget should expose a connection status text block"), ConnectionStatusTextProperty);

    const FObjectProperty* JoinAddressTextBoxProperty =
        FindFProperty<FObjectProperty>(WidgetClass, TEXT("JoinAddressTextBox"));
    TestNotNull(TEXT("HUD widget should expose a join address text box"), JoinAddressTextBoxProperty);

    UFunction* HostMatchFunction = WidgetClass->FindFunctionByName(TEXT("HandleHostMatchButtonClicked"));
    TestNotNull(TEXT("HUD widget should expose a host-match button handler"), HostMatchFunction);

    UFunction* JoinByIpFunction = WidgetClass->FindFunctionByName(TEXT("HandleJoinByIpButtonClicked"));
    TestNotNull(TEXT("HUD widget should expose a join-by-ip button handler"), JoinByIpFunction);

    UFunction* BuildHostTravelCommandFunction = WidgetClass->FindFunctionByName(TEXT("BuildHostTravelCommand"));
    TestNotNull(TEXT("HUD widget should expose a host travel command builder"), BuildHostTravelCommandFunction);

    UFunction* BuildJoinTravelCommandFunction = WidgetClass->FindFunctionByName(TEXT("BuildJoinTravelCommand"));
    TestNotNull(TEXT("HUD widget should expose a join travel command builder"), BuildJoinTravelCommandFunction);

    return ConnectionStatusTextProperty
        && JoinAddressTextBoxProperty
        && HostMatchFunction
        && JoinByIpFunction
        && BuildHostTravelCommandFunction
        && BuildJoinTravelCommandFunction;
}
