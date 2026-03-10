#include "Misc/AutomationTest.h"
#include "Player/PvPArenaCharacter.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterDeathAnimationDefaultsTest,
    "PvPArena.Character.DeathAnimationDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterDeathAnimationDefaultsTest::RunTest(const FString& Parameters)
{
    const APvPArenaCharacter* Character = GetDefault<APvPArenaCharacter>();
    TestNotNull(TEXT("Character CDO should exist"), Character);

    const FObjectProperty* DeathAnimationProperty = FindFProperty<FObjectProperty>(APvPArenaCharacter::StaticClass(), TEXT("DeathAnimation"));
    TestNotNull(TEXT("DeathAnimation property should exist"), DeathAnimationProperty);

    const UObject* DeathAnimation = (Character && DeathAnimationProperty)
        ? DeathAnimationProperty->GetObjectPropertyValue_InContainer(Character)
        : nullptr;

    TestNotNull(TEXT("Character should default to a death animation asset"), DeathAnimation);
    return true;
}
