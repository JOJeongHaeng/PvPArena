#if WITH_EDITOR

#include "Misc/AutomationTest.h"
#include "UI/PvPArenaHUDWidget.h"

namespace
{
constexpr TCHAR SettingsSection[] = TEXT("PvPArena.HUDUserSettings");
constexpr TCHAR MasterVolumeKey[] = TEXT("MasterVolume");
constexpr TCHAR BackgroundMusicVolumeKey[] = TEXT("BackgroundMusicVolume");
constexpr TCHAR SfxVolumeKey[] = TEXT("SfxVolume");

void RestoreStringSetting(const TCHAR* KeyName, const FString& PreviousValue, bool bHadValue)
{
    if (bHadValue)
    {
        GConfig->SetString(SettingsSection, KeyName, *PreviousValue, GGameUserSettingsIni);
    }
    else
    {
        GConfig->RemoveKey(SettingsSection, KeyName, GGameUserSettingsIni);
    }
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHUDWidgetPersistenceTest,
    "PvPArena.UI.HUDWidgetPersistence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDWidgetPersistenceTest::RunTest(const FString& Parameters)
{
    UFunction* LoadSettingsFunction = UPvPArenaHUDWidget::StaticClass()->FindFunctionByName(TEXT("LoadPersistentUserSettings"));
    UFunction* SaveSettingsFunction = UPvPArenaHUDWidget::StaticClass()->FindFunctionByName(TEXT("SavePersistentUserSettings"));
    UFunction* ReleaseAudioFunction = UPvPArenaHUDWidget::StaticClass()->FindFunctionByName(TEXT("ReleaseBackgroundMusicAudioComponent"));
    UFunction* MasterVolumeChangedFunction = UPvPArenaHUDWidget::StaticClass()->FindFunctionByName(TEXT("HandleMasterVolumeSliderChanged"));
    UFunction* BackgroundMusicVolumeChangedFunction = UPvPArenaHUDWidget::StaticClass()->FindFunctionByName(TEXT("HandleBgmVolumeSliderChanged"));
    UFunction* SfxVolumeChangedFunction = UPvPArenaHUDWidget::StaticClass()->FindFunctionByName(TEXT("HandleSfxVolumeSliderChanged"));
    TestNotNull(TEXT("HUD widget should expose a load-settings helper"), LoadSettingsFunction);
    TestNotNull(TEXT("HUD widget should expose a save-settings helper"), SaveSettingsFunction);
    TestNotNull(TEXT("HUD widget should expose an audio cleanup helper"), ReleaseAudioFunction);
    TestNotNull(TEXT("HUD widget should expose a master-volume slider handler"), MasterVolumeChangedFunction);
    TestNotNull(TEXT("HUD widget should expose a background-music slider handler"), BackgroundMusicVolumeChangedFunction);
    TestNotNull(TEXT("HUD widget should expose a sfx-volume slider handler"), SfxVolumeChangedFunction);

    if (!LoadSettingsFunction
        || !SaveSettingsFunction
        || !ReleaseAudioFunction
        || !MasterVolumeChangedFunction
        || !BackgroundMusicVolumeChangedFunction
        || !SfxVolumeChangedFunction)
    {
        return false;
    }

    FString PreviousMasterVolume;
    FString PreviousBackgroundMusicVolume;
    FString PreviousSfxVolume;
    const bool bHadMasterVolume = GConfig->GetString(SettingsSection, MasterVolumeKey, PreviousMasterVolume, GGameUserSettingsIni);
    const bool bHadBackgroundMusicVolume = GConfig->GetString(SettingsSection, BackgroundMusicVolumeKey, PreviousBackgroundMusicVolume, GGameUserSettingsIni);
    const bool bHadSfxVolume = GConfig->GetString(SettingsSection, SfxVolumeKey, PreviousSfxVolume, GGameUserSettingsIni);

    UPvPArenaHUDWidget* SavingWidget = NewObject<UPvPArenaHUDWidget>();
    UPvPArenaHUDWidget* LoadingWidget = NewObject<UPvPArenaHUDWidget>();
    TestNotNull(TEXT("Saving widget should be created"), SavingWidget);
    TestNotNull(TEXT("Loading widget should be created"), LoadingWidget);

    if (!SavingWidget || !LoadingWidget)
    {
        return false;
    }

    struct FSliderChangedParams
    {
        float NewValue;
    };

    FSliderChangedParams MasterParams{0.35f};
    FSliderChangedParams BackgroundMusicParams{0.45f};
    FSliderChangedParams SfxParams{0.55f};
    SavingWidget->ProcessEvent(MasterVolumeChangedFunction, &MasterParams);
    SavingWidget->ProcessEvent(BackgroundMusicVolumeChangedFunction, &BackgroundMusicParams);
    SavingWidget->ProcessEvent(SfxVolumeChangedFunction, &SfxParams);
    SavingWidget->ProcessEvent(SaveSettingsFunction, nullptr);

    LoadingWidget->ProcessEvent(LoadSettingsFunction, nullptr);

    TestEqual(
        TEXT("Saved master volume should be restored into a new widget instance"),
        LoadingWidget->GetMasterVolume(),
        0.35f);
    TestEqual(
        TEXT("Saved background music volume should be restored into a new widget instance"),
        LoadingWidget->GetBackgroundMusicVolume(),
        0.45f);
    TestEqual(
        TEXT("Saved sfx volume should be restored into a new widget instance"),
        LoadingWidget->GetSfxVolume(),
        0.55f);

    LoadingWidget->ProcessEvent(ReleaseAudioFunction, nullptr);

    RestoreStringSetting(MasterVolumeKey, PreviousMasterVolume, bHadMasterVolume);
    RestoreStringSetting(BackgroundMusicVolumeKey, PreviousBackgroundMusicVolume, bHadBackgroundMusicVolume);
    RestoreStringSetting(SfxVolumeKey, PreviousSfxVolume, bHadSfxVolume);
    GConfig->Flush(false, GGameUserSettingsIni);

    return true;
}

#endif
