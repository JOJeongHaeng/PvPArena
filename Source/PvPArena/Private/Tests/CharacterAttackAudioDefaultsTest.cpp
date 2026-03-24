#include "Misc/AutomationTest.h"
#include "Sound/SoundBase.h"
#include "Player/PvPArenaCharacter.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterAttackAudioDefaultsTest,
    "PvPArena.Character.AttackAudioDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterAttackAudioDefaultsTest::RunTest(const FString& Parameters)
{
    const FString ExpectedMeleeAttackSoundPath =
        TEXT("/Game/PvPArena/Audio/Fire01_Cue.Fire01_Cue");
    const FString ExpectedRangedAttackSoundPath =
        TEXT("/Game/PvPArena/Audio/Explosion_Cue.Explosion_Cue");

    const APvPArenaCharacter* Character = GetDefault<APvPArenaCharacter>();
    TestNotNull(TEXT("Character CDO should exist"), Character);

    const FObjectProperty* MeleeAttackSoundProperty =
        FindFProperty<FObjectProperty>(APvPArenaCharacter::StaticClass(), TEXT("MeleeAttackSound"));
    const FObjectProperty* RangedAttackSoundProperty =
        FindFProperty<FObjectProperty>(APvPArenaCharacter::StaticClass(), TEXT("RangedAttackSound"));
    const FFloatProperty* MeleeAttackSoundDurationProperty =
        FindFProperty<FFloatProperty>(APvPArenaCharacter::StaticClass(), TEXT("MeleeAttackSoundDurationSeconds"));
    const FFloatProperty* RangedAttackSoundVolumeProperty =
        FindFProperty<FFloatProperty>(APvPArenaCharacter::StaticClass(), TEXT("RangedAttackSoundVolume"));
    UFunction* RangedAttackSoundMulticast =
        APvPArenaCharacter::StaticClass()->FindFunctionByName(TEXT("MulticastPlayRangedAttackSound"));

    TestNotNull(TEXT("Character should expose a melee attack sound property"), MeleeAttackSoundProperty);
    TestNotNull(TEXT("Character should expose a ranged attack sound property"), RangedAttackSoundProperty);
    TestNotNull(TEXT("Character should expose a melee attack sound duration"), MeleeAttackSoundDurationProperty);
    TestNotNull(TEXT("Character should expose a ranged attack sound volume"), RangedAttackSoundVolumeProperty);
    TestNotNull(TEXT("Character should multicast ranged attack audio"), RangedAttackSoundMulticast);

    if (!Character || !MeleeAttackSoundProperty || !RangedAttackSoundProperty || !MeleeAttackSoundDurationProperty || !RangedAttackSoundVolumeProperty || !RangedAttackSoundMulticast)
    {
        return false;
    }

    const USoundBase* MeleeAttackSound = Cast<USoundBase>(MeleeAttackSoundProperty->GetObjectPropertyValue_InContainer(Character));
    const USoundBase* RangedAttackSound = Cast<USoundBase>(RangedAttackSoundProperty->GetObjectPropertyValue_InContainer(Character));

    TestNotNull(TEXT("Character should default to a melee attack sound"), MeleeAttackSound);
    TestNotNull(TEXT("Character should default to a ranged attack sound"), RangedAttackSound);
    TestEqual(
        TEXT("Character should use the project-owned melee fire cue"),
        MeleeAttackSound ? MeleeAttackSound->GetPathName() : FString(),
        ExpectedMeleeAttackSoundPath);
    TestEqual(
        TEXT("Character should use the project-owned ranged explosion cue"),
        RangedAttackSound ? RangedAttackSound->GetPathName() : FString(),
        ExpectedRangedAttackSoundPath);
    TestEqual(
        TEXT("Melee attack sound should play for about 1.25 seconds"),
        MeleeAttackSoundDurationProperty->GetPropertyValue_InContainer(Character),
        1.25f);
    TestEqual(
        TEXT("Ranged attack sound should play at 70 percent volume"),
        RangedAttackSoundVolumeProperty->GetPropertyValue_InContainer(Character),
        0.7f);
    TestTrue(
        TEXT("Ranged attack sound RPC should replicate to all clients"),
        (RangedAttackSoundMulticast->FunctionFlags & FUNC_NetMulticast) != 0);
    return true;
}
