#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PvPArenaGameInstance.generated.h"

UCLASS()
class PVPARENA_API UPvPArenaGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;

    void LoadUserSettings();
    void SaveUserSettings() const;
    void SetAudioSettings(float InMasterVolume, float InBackgroundMusicVolume, float InSfxVolume);

    float GetMasterVolume() const { return MasterVolume; }
    float GetBackgroundMusicVolume() const { return BackgroundMusicVolume; }
    float GetSfxVolume() const { return SfxVolume; }

private:
    float MasterVolume = 1.0f;
    float BackgroundMusicVolume = 1.0f;
    float SfxVolume = 1.0f;
};
