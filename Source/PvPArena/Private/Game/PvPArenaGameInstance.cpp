#include "Game/PvPArenaGameInstance.h"

#include "Misc/ConfigCacheIni.h"

namespace
{
const TCHAR* HUDUserSettingsSection = TEXT("PvPArena.HUDUserSettings");
const TCHAR* MasterVolumeSettingKey = TEXT("MasterVolume");
const TCHAR* BackgroundMusicVolumeSettingKey = TEXT("BackgroundMusicVolume");
const TCHAR* SfxVolumeSettingKey = TEXT("SfxVolume");
}

void UPvPArenaGameInstance::Init()
{
    Super::Init();
    LoadUserSettings();
}

void UPvPArenaGameInstance::LoadUserSettings()
{
    MasterVolume = 1.0f;
    BackgroundMusicVolume = 1.0f;
    SfxVolume = 1.0f;

    if (!GConfig)
    {
        return;
    }

    float PersistedValue = 1.0f;
    if (GConfig->GetFloat(HUDUserSettingsSection, MasterVolumeSettingKey, PersistedValue, GGameUserSettingsIni))
    {
        MasterVolume = FMath::Clamp(PersistedValue, 0.0f, 1.0f);
    }

    if (GConfig->GetFloat(HUDUserSettingsSection, BackgroundMusicVolumeSettingKey, PersistedValue, GGameUserSettingsIni))
    {
        BackgroundMusicVolume = FMath::Clamp(PersistedValue, 0.0f, 1.0f);
    }

    if (GConfig->GetFloat(HUDUserSettingsSection, SfxVolumeSettingKey, PersistedValue, GGameUserSettingsIni))
    {
        SfxVolume = FMath::Clamp(PersistedValue, 0.0f, 1.0f);
    }

}

void UPvPArenaGameInstance::SaveUserSettings() const
{
    if (!GConfig)
    {
        return;
    }

    GConfig->SetFloat(HUDUserSettingsSection, MasterVolumeSettingKey, MasterVolume, GGameUserSettingsIni);
    GConfig->SetFloat(HUDUserSettingsSection, BackgroundMusicVolumeSettingKey, BackgroundMusicVolume, GGameUserSettingsIni);
    GConfig->SetFloat(HUDUserSettingsSection, SfxVolumeSettingKey, SfxVolume, GGameUserSettingsIni);
    GConfig->Flush(false, GGameUserSettingsIni);

}

void UPvPArenaGameInstance::SetAudioSettings(float InMasterVolume, float InBackgroundMusicVolume, float InSfxVolume)
{
    MasterVolume = FMath::Clamp(InMasterVolume, 0.0f, 1.0f);
    BackgroundMusicVolume = FMath::Clamp(InBackgroundMusicVolume, 0.0f, 1.0f);
    SfxVolume = FMath::Clamp(InSfxVolume, 0.0f, 1.0f);
}
