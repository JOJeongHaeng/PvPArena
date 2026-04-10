#if WITH_EDITOR

#include "Components/AudioComponent.h"
#include "Misc/AutomationTest.h"
#include "UI/PvPArenaHUDWidget.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHUDWidgetAudioCleanupTest,
    "PvPArena.UI.HUDWidgetAudioCleanup",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDWidgetAudioCleanupTest::RunTest(const FString& Parameters)
{
    const FObjectProperty* BackgroundMusicAudioComponentProperty =
        FindFProperty<FObjectProperty>(UPvPArenaHUDWidget::StaticClass(), TEXT("BackgroundMusicAudioComponent"));
    UFunction* ReleaseAudioFunction = UPvPArenaHUDWidget::StaticClass()->FindFunctionByName(TEXT("ReleaseBackgroundMusicAudioComponent"));

    TestNotNull(TEXT("HUD widget should expose its background music audio component property"), BackgroundMusicAudioComponentProperty);
    TestNotNull(TEXT("HUD widget should expose an audio cleanup helper"), ReleaseAudioFunction);

    if (!BackgroundMusicAudioComponentProperty || !ReleaseAudioFunction)
    {
        return false;
    }

    UPvPArenaHUDWidget* Widget = NewObject<UPvPArenaHUDWidget>();
    UAudioComponent* BackgroundMusicComponent = NewObject<UAudioComponent>(GetTransientPackage());
    TestNotNull(TEXT("Cleanup test should create a HUD widget"), Widget);
    TestNotNull(TEXT("Cleanup test should create a background music audio component"), BackgroundMusicComponent);

    if (!Widget || !BackgroundMusicComponent)
    {
        return false;
    }

    BackgroundMusicAudioComponentProperty->SetObjectPropertyValue_InContainer(Widget, BackgroundMusicComponent);

    Widget->ProcessEvent(ReleaseAudioFunction, nullptr);

    TestNull(
        TEXT("Background music cleanup should clear the widget-owned audio component reference"),
        Widget->GetBackgroundMusicAudioComponent());
    TestFalse(
        TEXT("Background music cleanup should unregister the released component"),
        BackgroundMusicComponent->IsRegistered());
    TestFalse(
        TEXT("Background music cleanup should not explicitly destroy a widget-owned component"),
        BackgroundMusicComponent->IsBeingDestroyed());

    return true;
}

#endif
