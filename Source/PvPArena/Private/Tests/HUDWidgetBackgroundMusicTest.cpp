#include "Misc/AutomationTest.h"
#include "Game/PvPArenaGameState.h"
#include "Sound/SoundBase.h"
#include "UI/PvPArenaHUDWidget.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHUDWidgetBackgroundMusicTest,
    "PvPArena.UI.HUDWidgetBackgroundMusic",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHUDWidgetBackgroundMusicTest::RunTest(const FString& Parameters)
{
    const FString ExpectedNonCombatMusicPath =
        TEXT("/Game/PvPArena/Audio/Starter_Music_Cue.Starter_Music_Cue");
    const FString ExpectedGameplayMusicPath =
        TEXT("/Game/PvPArena/Audio/Starter_Background_Cue.Starter_Background_Cue");

    const UPvPArenaHUDWidget* Widget = GetDefault<UPvPArenaHUDWidget>();
    TestNotNull(TEXT("HUD widget CDO should exist"), Widget);

    const FObjectProperty* NonCombatMusicProperty =
        FindFProperty<FObjectProperty>(UPvPArenaHUDWidget::StaticClass(), TEXT("NonCombatBackgroundMusic"));
    const FObjectProperty* GameplayMusicProperty =
        FindFProperty<FObjectProperty>(UPvPArenaHUDWidget::StaticClass(), TEXT("GameplayBackgroundMusic"));

    TestNotNull(TEXT("HUD widget should expose a non-combat background music property"), NonCombatMusicProperty);
    TestNotNull(TEXT("HUD widget should expose a gameplay background music property"), GameplayMusicProperty);

    if (!Widget || !NonCombatMusicProperty || !GameplayMusicProperty)
    {
        return false;
    }

    const USoundBase* NonCombatMusic = Cast<USoundBase>(NonCombatMusicProperty->GetObjectPropertyValue_InContainer(Widget));
    const USoundBase* GameplayMusic = Cast<USoundBase>(GameplayMusicProperty->GetObjectPropertyValue_InContainer(Widget));

    TestNotNull(TEXT("HUD widget should default to a non-combat background music cue"), NonCombatMusic);
    TestNotNull(TEXT("HUD widget should default to a gameplay background music cue"), GameplayMusic);
    TestEqual(
        TEXT("HUD widget should use the project-owned non-combat music cue"),
        NonCombatMusic ? NonCombatMusic->GetPathName() : FString(),
        ExpectedNonCombatMusicPath);
    TestEqual(
        TEXT("HUD widget should use the project-owned gameplay music cue"),
        GameplayMusic ? GameplayMusic->GetPathName() : FString(),
        ExpectedGameplayMusicPath);

    TestEqual(
        TEXT("Lobby should resolve to non-combat music"),
        UPvPArenaHUDWidget::BuildBackgroundMusicAssetPathForMatchPhase(static_cast<uint8>(EPvPAMatchPhase::Lobby)),
        ExpectedNonCombatMusicPath);
    TestEqual(
        TEXT("Playing should resolve to gameplay music"),
        UPvPArenaHUDWidget::BuildBackgroundMusicAssetPathForMatchPhase(static_cast<uint8>(EPvPAMatchPhase::Playing)),
        ExpectedGameplayMusicPath);
    TestEqual(
        TEXT("MatchEnd should keep the non-combat music cue"),
        UPvPArenaHUDWidget::BuildBackgroundMusicAssetPathForMatchPhase(static_cast<uint8>(EPvPAMatchPhase::MatchEnd)),
        ExpectedNonCombatMusicPath);

    return true;
}
