#include "Misc/AutomationTest.h"
#include "UI/PvPArenaHUDWidget.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHUDWidgetSettingsMenuTest,
    "PvPArena.UI.HUDWidgetSettingsMenu",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDWidgetSettingsMenuTest::RunTest(const FString& Parameters)
{
    const UClass* WidgetClass = UPvPArenaHUDWidget::StaticClass();

    const FObjectProperty* SettingsPanelProperty =
        FindFProperty<FObjectProperty>(WidgetClass, TEXT("SettingsPanel"));
    TestNotNull(TEXT("HUD widget should expose a settings panel"), SettingsPanelProperty);

    const FObjectProperty* SettingsResumeButtonProperty =
        FindFProperty<FObjectProperty>(WidgetClass, TEXT("SettingsResumeButton"));
    TestNotNull(TEXT("HUD widget should expose a resume button"), SettingsResumeButtonProperty);

    const FObjectProperty* SettingsQuitButtonProperty =
        FindFProperty<FObjectProperty>(WidgetClass, TEXT("SettingsQuitButton"));
    TestNotNull(TEXT("HUD widget should expose a quit button"), SettingsQuitButtonProperty);

    UFunction* ToggleSettingsFunction = WidgetClass->FindFunctionByName(TEXT("ToggleSettingsMenu"));
    TestNotNull(TEXT("HUD widget should expose a settings menu toggle"), ToggleSettingsFunction);

    UFunction* ResumeFunction = WidgetClass->FindFunctionByName(TEXT("HandleSettingsResumeButtonClicked"));
    TestNotNull(TEXT("HUD widget should expose a resume handler"), ResumeFunction);

    UFunction* QuitFunction = WidgetClass->FindFunctionByName(TEXT("HandleSettingsQuitButtonClicked"));
    TestNotNull(TEXT("HUD widget should expose a quit handler"), QuitFunction);

    UFunction* ApplyDisplayFunction = WidgetClass->FindFunctionByName(TEXT("ApplyDisplaySettings"));
    TestNotNull(TEXT("HUD widget should expose a display settings apply helper"), ApplyDisplayFunction);

    UFunction* ApplyAudioFunction = WidgetClass->FindFunctionByName(TEXT("ApplyAudioSettings"));
    TestNotNull(TEXT("HUD widget should expose an audio settings apply helper"), ApplyAudioFunction);

    return SettingsPanelProperty
        && SettingsResumeButtonProperty
        && SettingsQuitButtonProperty
        && ToggleSettingsFunction
        && ResumeFunction
        && QuitFunction
        && ApplyDisplayFunction
        && ApplyAudioFunction;
}
