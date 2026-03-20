#include "Misc/AutomationTest.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/PvPArenaCharacter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterSprintStateTest,
    "PvPArena.Character.SprintState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterSprintStateTest::RunTest(const FString& Parameters)
{
    APvPArenaCharacter* Character = NewObject<APvPArenaCharacter>();
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

    const float BaseWalkSpeed = MoveComp->MaxWalkSpeed;
    TestEqual(TEXT("Sprint energy should start full"), Character->GetCurrentSprintEnergySeconds(), Character->GetSprintDurationSeconds());
    TestEqual(TEXT("Sprint gauge should start full"), Character->GetSprintEnergyAlpha(), 1.0f);
    TestFalse(TEXT("Sprint should start inactive"), Character->IsSprinting());
    TestEqual(TEXT("Sprint duration should use the tuned one point five seconds"), Character->GetSprintDurationSeconds(), 1.5f);
    TestEqual(TEXT("Sprint recharge rate should be slower than drain"), Character->GetSprintRechargeRate(), 0.75f);

    Character->BeginSprintInput();
    Character->UpdateSprintState(1.0f);

    TestTrue(TEXT("Sprint should activate while input is held"), Character->IsSprinting());
    TestEqual(TEXT("Sprint should spend one second of energy per second"), Character->GetCurrentSprintEnergySeconds(), 0.5f);
    TestEqual(TEXT("Sprint should raise walk speed by the configured multiplier"), MoveComp->MaxWalkSpeed, BaseWalkSpeed * Character->GetSprintSpeedMultiplier());

    Character->UpdateSprintState(0.5f);

    TestFalse(TEXT("Sprint should stop when the gauge is empty"), Character->IsSprinting());
    TestEqual(TEXT("Sprint energy should empty after the full duration"), Character->GetCurrentSprintEnergySeconds(), 0.0f);
    TestEqual(TEXT("Walk speed should return to base when sprint ends"), MoveComp->MaxWalkSpeed, BaseWalkSpeed);

    Character->BeginSprintInput();
    Character->UpdateSprintState(0.5f);

    TestFalse(TEXT("Sprint should stay locked while the gauge is empty"), Character->IsSprinting());
    TestEqual(TEXT("Locked sprint should keep the energy empty until recharge starts"), Character->GetCurrentSprintEnergySeconds(), 0.375f);

    Character->EndSprintInput();
    Character->UpdateSprintState(1.0f);

    TestEqual(TEXT("Sprint should recharge more slowly than it drains"), Character->GetCurrentSprintEnergySeconds(), 1.125f);
    TestEqual(TEXT("Sprint gauge alpha should follow the current partial charge"), Character->GetSprintEnergyAlpha(), 0.75f);

    Character->UpdateSprintState(0.5f);
    Character->BeginSprintInput();
    Character->UpdateSprintState(0.25f);

    TestTrue(TEXT("Sprint should become usable again after a full recharge"), Character->IsSprinting());
    return true;
}
