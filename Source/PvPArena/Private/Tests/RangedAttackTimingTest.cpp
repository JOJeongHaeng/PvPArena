#include "Misc/AutomationTest.h"
#include "Player/PvPArenaCharacter.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRangedAttackTimingTest,
    "PvPArena.Combat.RangedAttackTiming",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

namespace
{
}

bool FRangedAttackTimingTest::RunTest(const FString& Parameters)
{
    APvPArenaCharacter* Character = NewObject<APvPArenaCharacter>();
    TestNotNull(TEXT("Character should be created"), Character);

    if (!Character)
    {
        return false;
    }

    UClass* CharacterClass = Character->GetClass();
    const FBoolProperty* MovementSuppressedProperty = FindFProperty<FBoolProperty>(CharacterClass, TEXT("bAttackMovementSuppressed"));
    const FBoolProperty* FacingLockedProperty = FindFProperty<FBoolProperty>(CharacterClass, TEXT("bRangedAttackFacingLocked"));
    const FBoolProperty* InputHeldProperty = FindFProperty<FBoolProperty>(CharacterClass, TEXT("bRangedChargeInputHeld"));
    const FBoolProperty* ReleaseCommittedProperty = FindFProperty<FBoolProperty>(CharacterClass, TEXT("bRangedReleaseCommitted"));
    const FFloatProperty* FacingTargetYawProperty = FindFProperty<FFloatProperty>(CharacterClass, TEXT("RangedAttackTargetYaw"));
    const FFloatProperty* ChargeStartTimeProperty = FindFProperty<FFloatProperty>(CharacterClass, TEXT("RangedChargeStartTime"));
    const FFloatProperty* CameraBlendAlphaProperty = FindFProperty<FFloatProperty>(CharacterClass, TEXT("RangedAimCameraBlendAlpha"));
    const FFloatProperty* MinimumHoldSecondsProperty = FindFProperty<FFloatProperty>(CharacterClass, TEXT("RangedChargeMinimumHoldSeconds"));
    TestNotNull(TEXT("Character should track shared attack movement suppression"), MovementSuppressedProperty);
    TestNotNull(TEXT("Character should lock ranged facing while attacking"), FacingLockedProperty);
    TestNotNull(TEXT("Character should track whether ranged charge input is still held"), InputHeldProperty);
    TestNotNull(TEXT("Character should track whether ranged release was committed"), ReleaseCommittedProperty);
    TestNotNull(TEXT("Character should store the ranged target yaw"), FacingTargetYawProperty);
    TestNotNull(TEXT("Character should store ranged charge start time"), ChargeStartTimeProperty);
    TestNotNull(TEXT("Character should track ranged aim camera blend alpha"), CameraBlendAlphaProperty);
    TestNotNull(TEXT("Character should expose ranged minimum hold seconds"), MinimumHoldSecondsProperty);

    if (!MovementSuppressedProperty || !FacingLockedProperty || !InputHeldProperty || !ReleaseCommittedProperty || !FacingTargetYawProperty || !ChargeStartTimeProperty || !CameraBlendAlphaProperty || !MinimumHoldSecondsProperty)
    {
        return false;
    }

    TestNotNull(TEXT("Character should create a combat component"), Character->GetCombatComponent());
    TestFalse(TEXT("Ranged attack should not start in progress"), Character->IsRangedAttackInProgress());
    TestFalse(TEXT("Ranged hit should not start triggered"), Character->HasTriggeredRangedAttackHit());
    TestFalse(TEXT("Ranged charge input should start released"), InputHeldProperty->GetPropertyValue_InContainer(Character));
    TestFalse(TEXT("Ranged release should start uncommitted"), ReleaseCommittedProperty->GetPropertyValue_InContainer(Character));
    TestFalse(TEXT("Movement suppression should start disabled"), MovementSuppressedProperty->GetPropertyValue_InContainer(Character));
    TestFalse(TEXT("Facing lock should start disabled"), FacingLockedProperty->GetPropertyValue_InContainer(Character));
    TestEqual(TEXT("Ranged aim camera blend should start disabled"), CameraBlendAlphaProperty->GetPropertyValue_InContainer(Character), 0.0f);
    TestEqual(TEXT("Ranged charge minimum hold should now be half a second"), MinimumHoldSecondsProperty->GetPropertyValue_InContainer(Character), 0.5f);

    Character->SetActorRotation(FRotator::ZeroRotator);
    TestTrue(TEXT("First ranged charge start should succeed"), Character->BeginRangedCharge(0.0f));
    TestTrue(TEXT("Ranged charge should enter in-progress state"), Character->IsRangedAttackInProgress());
    TestTrue(TEXT("Starting a ranged charge should track held input"), InputHeldProperty->GetPropertyValue_InContainer(Character));
    TestFalse(TEXT("Starting a ranged charge should not immediately commit release"), ReleaseCommittedProperty->GetPropertyValue_InContainer(Character));
    TestFalse(TEXT("Starting a ranged charge should not immediately trigger the hit"), Character->HasTriggeredRangedAttackHit());
    TestEqual(TEXT("Ranged charge should record its start time"), ChargeStartTimeProperty->GetPropertyValue_InContainer(Character), 0.0f);
    TestTrue(TEXT("Starting a ranged charge should suppress movement"), MovementSuppressedProperty->GetPropertyValue_InContainer(Character));
    TestTrue(TEXT("Starting a ranged charge should lock facing"), FacingLockedProperty->GetPropertyValue_InContainer(Character));

    FacingTargetYawProperty->SetPropertyValue_InContainer(Character, 90.0f);
    Character->AdvanceAttackFacing(0.1f);
    TestTrue(TEXT("Tick should rotate the character toward the locked ranged yaw"), Character->GetActorRotation().Yaw > 0.0f);
    TestTrue(TEXT("Holding ranged charge should push aim camera blend up"), CameraBlendAlphaProperty->GetPropertyValue_InContainer(Character) > 0.0f);

    TestFalse(TEXT("A second ranged charge start while active should be blocked"), Character->BeginRangedCharge(0.1f));

    TestFalse(TEXT("Releasing before the threshold should cancel the ranged charge"), Character->ReleaseRangedCharge(0.25f));
    TestFalse(TEXT("Early release should not commit the ranged attack"), ReleaseCommittedProperty->GetPropertyValue_InContainer(Character));
    TestFalse(TEXT("Early release should not trigger the hit"), Character->HasTriggeredRangedAttackHit());
    Character->AdvanceAttackFacing(0.1f);
    TestTrue(TEXT("Cancel release should start returning aim camera blend"), CameraBlendAlphaProperty->GetPropertyValue_InContainer(Character) < 1.0f);

    Character->FinishRangedAttack();
    TestFalse(TEXT("Cancel finish should clear ranged in-progress state"), Character->IsRangedAttackInProgress());

    TestTrue(TEXT("A new ranged charge should start after cancel"), Character->BeginRangedCharge(2.0f));
    TestTrue(TEXT("Releasing after the threshold should commit the ranged charge"), Character->ReleaseRangedCharge(3.1f));
    TestTrue(TEXT("Late release should commit the ranged attack"), ReleaseCommittedProperty->GetPropertyValue_InContainer(Character));

    Character->HandleRangedAttackHitNotify();
    TestTrue(TEXT("Ranged hit should be marked triggered after notify"), Character->HasTriggeredRangedAttackHit());
    TestFalse(TEXT("Ranged notify should not trigger twice"), Character->HandleRangedAttackHitNotify());

    Character->FinishRangedAttack();
    TestFalse(TEXT("Finish should clear ranged in-progress state"), Character->IsRangedAttackInProgress());
    TestFalse(TEXT("Finish should clear ranged hit-triggered state"), Character->HasTriggeredRangedAttackHit());
    TestFalse(TEXT("Finish should clear movement suppression"), MovementSuppressedProperty->GetPropertyValue_InContainer(Character));
    TestFalse(TEXT("Finish should clear facing lock"), FacingLockedProperty->GetPropertyValue_InContainer(Character));

    return true;
}
