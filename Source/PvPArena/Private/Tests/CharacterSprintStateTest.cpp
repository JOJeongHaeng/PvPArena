#include "Misc/AutomationTest.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Player/PvPArenaCharacter.h"

namespace
{
class ATestPvPArenaCharacter : public APvPArenaCharacter
{
public:
    void SetTestController(AController* InController)
    {
        Controller = InController;
    }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterSprintStateTest,
    "PvPArena.Character.SprintState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterSprintStateTest::RunTest(const FString& Parameters)
{
    ATestPvPArenaCharacter* Character = NewObject<ATestPvPArenaCharacter>();
    TestNotNull(TEXT("Character should be created"), Character);

    if (!Character)
    {
        return false;
    }

    UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
    TestNotNull(TEXT("Character movement should exist"), MoveComp);

    if (!MoveComp)
    {
        return false;
    }

    APlayerController* TestController = NewObject<APlayerController>();
    TestNotNull(TEXT("Test controller should be created"), TestController);
    if (!TestController)
    {
        return false;
    }

    Character->SetTestController(TestController);
    Character->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
    TestController->SetControlRotation(FRotator(0.0f, 90.0f, 0.0f));

    TestEqual(TEXT("Sprint energy should start full"), Character->GetCurrentSprintEnergySeconds(), Character->GetSprintDurationSeconds());
    TestEqual(TEXT("Sprint gauge should start full"), Character->GetSprintEnergyAlpha(), 1.0f);
    TestFalse(TEXT("Sprint should start inactive"), Character->IsSprinting());
    TestEqual(TEXT("Sprint duration should use the tuned one point five seconds"), Character->GetSprintDurationSeconds(), 1.5f);
    TestEqual(TEXT("Sprint recharge rate should be slower than drain"), Character->GetSprintRechargeRate(), 0.75f);

    Character->BeginSprintInput();

    TestFalse(TEXT("Unaligned idle dash should not launch before the body turns"), Character->IsSprinting());
    TestEqual(TEXT("Pending preturn dash should not spend charge before launch"), Character->GetCurrentSprintEnergySeconds(), Character->GetSprintDurationSeconds());
    TestTrue(TEXT("Pending preturn dash should start without horizontal velocity"), MoveComp->Velocity.IsNearlyZero());

    Character->UpdateSprintState(0.05f);

    TestTrue(TEXT("Preturn should rotate the actor toward the camera yaw"), Character->GetActorRotation().Yaw > 0.0f);
    TestTrue(TEXT("Preturn should still avoid launching until alignment is close"), MoveComp->Velocity.IsNearlyZero());

    Character->UpdateSprintState(0.20f);

    TestTrue(TEXT("Dash should become active once the preturn finishes"), Character->IsSprinting());
    TestTrue(TEXT("Dash should create horizontal velocity after the preturn"), MoveComp->Velocity.Size2D() > 0.0f);
    TestTrue(TEXT("Dash should follow the controller-facing yaw after turning"), MoveComp->Velocity.Y > 0.0f);
    TestEqual(TEXT("Dash should spend the full charge on activation"), Character->GetCurrentSprintEnergySeconds(), 0.0f);
    TestEqual(TEXT("Dash gauge should empty when the charge is spent"), Character->GetSprintEnergyAlpha(), 0.0f);

    const FVector FirstDashVelocity = MoveComp->Velocity;
    Character->BeginSprintInput();

    TestEqual(TEXT("Empty dash should not fire again before recharge"), MoveComp->Velocity, FirstDashVelocity);
    TestEqual(TEXT("Empty dash should keep the charge at zero"), Character->GetCurrentSprintEnergySeconds(), 0.0f);

    Character->EndSprintInput();
    Character->UpdateSprintState(1.0f);

    TestEqual(TEXT("Dash should recharge more slowly than the old drain window"), Character->GetCurrentSprintEnergySeconds(), 0.75f);
    TestEqual(TEXT("Dash gauge alpha should match the partial recharge"), Character->GetSprintEnergyAlpha(), 0.5f);

    Character->UpdateSprintState(1.0f);

    TestEqual(TEXT("Dash should fully recharge after enough time"), Character->GetCurrentSprintEnergySeconds(), Character->GetSprintDurationSeconds());

    Character->BeginSprintInput();

    TestTrue(TEXT("Dash should become usable again after a full recharge"), Character->IsSprinting());
    TestTrue(TEXT("Recharge completion should allow a fresh dash launch"), MoveComp->Velocity.Size2D() > 0.0f);
    return true;
}
