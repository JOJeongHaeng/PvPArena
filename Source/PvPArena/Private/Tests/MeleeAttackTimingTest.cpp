  #include "Misc/AutomationTest.h"
  #include "GameFramework/CharacterMovementComponent.h"
  #include "GameFramework/Pawn.h"
  #include "GameFramework/PlayerController.h"
  #include "Player/PvPArenaCharacter.h"
  #include "UObject/Class.h"
  #include "UObject/UnrealType.h"

  IMPLEMENT_SIMPLE_AUTOMATION_TEST(
      FMeleeAttackTimingTest,
      "PvPArena.Combat.MeleeAttackTiming",
      EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

  bool FMeleeAttackTimingTest::RunTest(const FString& Parameters)
  {
      APvPArenaCharacter* Character = NewObject<APvPArenaCharacter>();
      TestNotNull(TEXT("Character should be created"), Character);

      if (!Character)
      {
          return false;
      }

      UClass* CharacterClass = Character->GetClass();
      const FBoolProperty* MovementSuppressedProperty =
          FindFProperty<FBoolProperty>(CharacterClass, TEXT("bAttackMovementSuppressed"));
      const FFloatProperty* MeleeAttackPlayRateProperty =
          FindFProperty<FFloatProperty>(CharacterClass, TEXT("MeleeAttackPlayRate"));
      const FObjectPropertyBase* ControllerProperty =
          FindFProperty<FObjectPropertyBase>(APawn::StaticClass(), TEXT("Controller"));

      TestNotNull(TEXT("Character should still expose attack movement suppression state"), MovementSuppressedProperty);
      TestNotNull(TEXT("Character should expose melee attack play rate"), MeleeAttackPlayRateProperty);
      TestNotNull(TEXT("Test should be able to assign a controller"), ControllerProperty);

      if (!MovementSuppressedProperty || !MeleeAttackPlayRateProperty || !ControllerProperty)
      {
          return false;
      }

      TestFalse(TEXT("Attack should not start in progress"), Character->IsMeleeAttackInProgress());
      TestFalse(TEXT("Hit should not start triggered"), Character->HasTriggeredMeleeAttackHit());
      TestFalse(
          TEXT("Attack movement suppression should start disabled"),
          MovementSuppressedProperty->GetPropertyValue_InContainer(Character));
      TestTrue(
          TEXT("Melee attack play rate should be faster than the old default"),
          MeleeAttackPlayRateProperty->GetPropertyValue_InContainer(Character) > 2.0f);

      APlayerController* Controller = NewObject<APlayerController>(Character);
      TestNotNull(TEXT("Controller should be created for melee facing checks"), Controller);
      if (!Controller)
      {
          return false;
      }

      Controller->SetControlRotation(FRotator(0.0f, 90.0f, 0.0f));
      ControllerProperty->SetObjectPropertyValue_InContainer(Character, Controller);

      Character->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
      TestFalse(TEXT("Melee attack should be blocked while airborne"), Character->BeginMeleeAttack(0.0f));
      TestFalse(TEXT("Airborne melee block should not start the attack"), Character->IsMeleeAttackInProgress());
      Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

      TestTrue(TEXT("First melee start should succeed"), Character->BeginMeleeAttack(0.0f));
      TestTrue(TEXT("Attack should enter in-progress state"), Character->IsMeleeAttackInProgress());
      TestFalse(
        TEXT("Starting an attack should not immediately trigger the hit"),
        Character->HasTriggeredMeleeAttackHit());
      TestFalse(
          TEXT("Melee attack should no longer suppress movement while active"),
          MovementSuppressedProperty->GetPropertyValue_InContainer(Character));

      Character->AdvanceAttackFacing(1.0f);
      TestTrue(
          TEXT("Melee attack should turn the actor toward controller yaw"),
          FMath::IsNearlyEqual(static_cast<float>(Character->GetActorRotation().Yaw), 90.0f, 0.01f));

      TestFalse(TEXT("A second start while active should be blocked"), Character->BeginMeleeAttack(0.1f));

      Character->HandleMeleeAttackHitNotify();
      TestTrue(TEXT("Hit should be marked triggered after notify"), Character->HasTriggeredMeleeAttackHit());
      TestFalse(TEXT("Notify should not trigger twice"), Character->HandleMeleeAttackHitNotify());

      Character->FinishMeleeAttack();
      TestFalse(TEXT("Finish should clear in-progress state"), Character->IsMeleeAttackInProgress());
      TestFalse(TEXT("Finish should clear hit-triggered state"), Character->HasTriggeredMeleeAttackHit());
      TestFalse(
          TEXT("Finish should keep attack movement suppression disabled"),
          MovementSuppressedProperty->GetPropertyValue_InContainer(Character));

      return true;
  }