#if WITH_EDITOR

#include "Game/PvPArenaGameInstance.h"
#include "Misc/AutomationTest.h"

namespace GameInstanceAudioSettingsTestInternal
{
constexpr TCHAR AudioSettingsSection[] = TEXT("PvPArena.HUDUserSettings");
constexpr TCHAR AudioMasterVolumeKey[] = TEXT("MasterVolume");
constexpr TCHAR AudioBackgroundMusicVolumeKey[] = TEXT("BackgroundMusicVolume");
constexpr TCHAR AudioSfxVolumeKey[] = TEXT("SfxVolume");

void RestoreAudioStringSetting(const TCHAR* KeyName, const FString& PreviousValue, bool bHadValue)
{
    if (bHadValue)
    {
        GConfig->SetString(AudioSettingsSection, KeyName, *PreviousValue, GGameUserSettingsIni);
    }
    else
    {
        GConfig->RemoveKey(AudioSettingsSection, KeyName, GGameUserSettingsIni);
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
    const bool bHadMasterVolume = GConfig->GetString(
        GameInstanceAudioSettingsTestInternal::AudioSettingsSection,
        GameInstanceAudioSettingsTestInternal::AudioMasterVolumeKey,
        PreviousMasterVolume,
        GGameUserSettingsIni);
    const bool bHadBackgroundMusicVolume = GConfig->GetString(
        GameInstanceAudioSettingsTestInternal::AudioSettingsSection,
        GameInstanceAudioSettingsTestInternal::AudioBackgroundMusicVolumeKey,
        PreviousBackgroundMusicVolume,
        GGameUserSettingsIni);
    const bool bHadSfxVolume = GConfig->GetString(
        GameInstanceAudioSettingsTestInternal::AudioSettingsSection,
        GameInstanceAudioSettingsTestInternal::AudioSfxVolumeKey,
        PreviousSfxVolume,
        GGameUserSettingsIni);

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

    GameInstanceAudioSettingsTestInternal::RestoreAudioStringSetting(
        GameInstanceAudioSettingsTestInternal::AudioMasterVolumeKey,
        PreviousMasterVolume,
        bHadMasterVolume);
    GameInstanceAudioSettingsTestInternal::RestoreAudioStringSetting(
        GameInstanceAudioSettingsTestInternal::AudioBackgroundMusicVolumeKey,
        PreviousBackgroundMusicVolume,
        bHadBackgroundMusicVolume);
    GameInstanceAudioSettingsTestInternal::RestoreAudioStringSetting(
        GameInstanceAudioSettingsTestInternal::AudioSfxVolumeKey,
        PreviousSfxVolume,
        bHadSfxVolume);
    GConfig->Flush(false, GGameUserSettingsIni);

    return true;
}

#endif
