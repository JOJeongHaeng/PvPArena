#if WITH_EDITOR

#include "Game/PvPArenaGameInstance.h"
#include "Misc/AutomationTest.h"

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
    FGameInstanceAudioSettingsTest,
    "PvPArena.GameInstance.AudioSettingsPersistence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGameInstanceAudioSettingsTest::RunTest(const FString& Parameters)
{
    FString PreviousMasterVolume;
    FString PreviousBackgroundMusicVolume;
    FString PreviousSfxVolume;
    const bool bHadMasterVolume = GConfig->GetString(SettingsSection, MasterVolumeKey, PreviousMasterVolume, GGameUserSettingsIni);
    const bool bHadBackgroundMusicVolume = GConfig->GetString(SettingsSection, BackgroundMusicVolumeKey, PreviousBackgroundMusicVolume, GGameUserSettingsIni);
    const bool bHadSfxVolume = GConfig->GetString(SettingsSection, SfxVolumeKey, PreviousSfxVolume, GGameUserSettingsIni);

    UPvPArenaGameInstance* SavingInstance = NewObject<UPvPArenaGameInstance>();
    UPvPArenaGameInstance* LoadingInstance = NewObject<UPvPArenaGameInstance>();
    TestNotNull(TEXT("Saving game instance should be created"), SavingInstance);
    TestNotNull(TEXT("Loading game instance should be created"), LoadingInstance);

    if (!SavingInstance || !LoadingInstance)
    {
        return false;
    }

    SavingInstance->SetAudioSettings(0.35f, 0.45f, 0.55f);
    SavingInstance->SaveUserSettings();

    LoadingInstance->LoadUserSettings();

    TestEqual(TEXT("Master volume should persist through game instance storage"), LoadingInstance->GetMasterVolume(), 0.35f);
    TestEqual(TEXT("Background music volume should persist through game instance storage"), LoadingInstance->GetBackgroundMusicVolume(), 0.45f);
    TestEqual(TEXT("Sfx volume should persist through game instance storage"), LoadingInstance->GetSfxVolume(), 0.55f);

    RestoreStringSetting(MasterVolumeKey, PreviousMasterVolume, bHadMasterVolume);
    RestoreStringSetting(BackgroundMusicVolumeKey, PreviousBackgroundMusicVolume, bHadBackgroundMusicVolume);
    RestoreStringSetting(SfxVolumeKey, PreviousSfxVolume, bHadSfxVolume);
    GConfig->Flush(false, GGameUserSettingsIni);

    return true;
}

#endif
