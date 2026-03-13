#include "Misc/AutomationTest.h"
#include "Player/PvPArenaCharacter.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterDeathAnimationDefaultsTest,
    "PvPArena.Character.DeathAnimationDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterDeathAnimationDefaultsTest::RunTest(const FString& Parameters)
{
    const FString ExpectedDeathAnimationPath = TEXT("/Game/PvPArena/Animations/Stand_Relaxed_Death.Stand_Relaxed_Death");
    const APvPArenaCharacter* Character = GetDefault<APvPArenaCharacter>();
    TestNotNull(TEXT("Character CDO should exist"), Character);

    const FObjectProperty* DeathAnimationProperty = FindFProperty<FObjectProperty>(APvPArenaCharacter::StaticClass(), TEXT("DeathAnimation"));
    TestNotNull(TEXT("DeathAnimation property should exist"), DeathAnimationProperty);

    const UObject* DeathAnimation = (Character && DeathAnimationProperty)
        ? DeathAnimationProperty->GetObjectPropertyValue_InContainer(Character)
        : nullptr;

    TestNotNull(TEXT("Character should default to a death animation asset"), DeathAnimation);
    TestEqual(
        TEXT("Character should use the project-owned death animation asset"),
        DeathAnimation ? DeathAnimation->GetPathName() : FString(),
        ExpectedDeathAnimationPath);
    return true;
}
