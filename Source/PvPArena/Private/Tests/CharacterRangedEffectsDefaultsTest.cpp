#include "Misc/AutomationTest.h"
#include "NiagaraSystem.h"
#include "Player/PvPArenaCharacter.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterRangedEffectsDefaultsTest,
    "PvPArena.Character.MeleeEffectsDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterRangedEffectsDefaultsTest::RunTest(const FString& Parameters)
{
    const FString ExpectedMeleeEffectPath =
        TEXT("/Game/PvPArena/VFX/fire/VFX/NS_Magma_Shot_Owner_Cast_Spell.NS_Magma_Shot_Owner_Cast_Spell");
    const APvPArenaCharacter* Character = GetDefault<APvPArenaCharacter>();
    TestNotNull(TEXT("Character CDO should exist"), Character);

    const FObjectProperty* MeleeEffectProperty =
        FindFProperty<FObjectProperty>(APvPArenaCharacter::StaticClass(), TEXT("MeleeAttackEffect"));
    TestNotNull(TEXT("Character should expose a melee attack effect property"), MeleeEffectProperty);

    const UObject* MeleeEffect = (Character && MeleeEffectProperty)
        ? MeleeEffectProperty->GetObjectPropertyValue_InContainer(Character)
        : nullptr;

    TestNotNull(TEXT("Character should default to a melee attack effect"), MeleeEffect);
    TestEqual(
        TEXT("Character should use the project-owned melee effect"),
        MeleeEffect ? MeleeEffect->GetPathName() : FString(),
        ExpectedMeleeEffectPath);
    return true;
}
