#include "Misc/AutomationTest.h"
#include "GameFramework/Pawn.h"
#include "Game/PvPArenaPlayerController.h"
#include "Player/PvPArenaCharacter.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterDeathInputSuppressionTest,
    "PvPArena.Character.DeathInputSuppression",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterDeathInputSuppressionTest::RunTest(const FString& Parameters)
{
    APvPArenaCharacter* Character = NewObject<APvPArenaCharacter>();
    APvPArenaPlayerController* PlayerController = NewObject<APvPArenaPlayerController>();

    TestNotNull(TEXT("Character should be created"), Character);
    TestNotNull(TEXT("PlayerController should be created"), PlayerController);

    const FObjectProperty* ControllerProperty = FindFProperty<FObjectProperty>(APawn::StaticClass(), TEXT("Controller"));
    TestNotNull(TEXT("Pawn Controller property should exist"), ControllerProperty);

    if (!Character || !PlayerController || !ControllerProperty)
    {
        return false;
    }

    ControllerProperty->SetObjectPropertyValue_InContainer(Character, PlayerController);

    TestFalse(TEXT("Move input should start enabled"), PlayerController->IsMoveInputIgnored());
    TestFalse(TEXT("Look input should start enabled"), PlayerController->IsLookInputIgnored());

    Character->ApplyServerDamage(150.0f, nullptr);

    TestTrue(TEXT("Move input should be ignored after lethal damage"), PlayerController->IsMoveInputIgnored());
    TestTrue(TEXT("Look input should be ignored after lethal damage"), PlayerController->IsLookInputIgnored());
    return true;
}
