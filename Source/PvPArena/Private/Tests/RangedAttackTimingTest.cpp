#include "Misc/AutomationTest.h"
#include "Camera/PlayerCameraManager.h"
#include "Combat/PvPCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Player/PvPArenaCharacter.h"
#include "UObject/Class.h"
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
    struct FServerHandleRangedAttackHitNotifyParams
    {
        bool bHasAim = false;
        FVector_NetQuantize AimOrigin = FVector::ZeroVector;
        FVector_NetQuantize AimTarget = FVector::ZeroVector;
    };

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
    const FBoolProperty* CachedAimProperty = FindFProperty<FBoolProperty>(CharacterClass, TEXT("bHasCachedRangedAttackAim"));
    const FStructProperty* CachedAimOriginProperty = FindFProperty<FStructProperty>(CharacterClass, TEXT("CachedRangedAttackAimOrigin"));
    const FStructProperty* CachedAimTargetProperty = FindFProperty<FStructProperty>(CharacterClass, TEXT("CachedRangedAttackAimTarget"));
    const FObjectPropertyBase* ControllerProperty = FindFProperty<FObjectPropertyBase>(APawn::StaticClass(), TEXT("Controller"));
    const FObjectPropertyBase* CameraManagerProperty = FindFProperty<FObjectPropertyBase>(APlayerController::StaticClass(), TEXT("PlayerCameraManager"));
    TestNotNull(TEXT("Character should track shared attack movement suppression"), MovementSuppressedProperty);
    TestNotNull(TEXT("Character should lock ranged facing while attacking"), FacingLockedProperty);
    TestNotNull(TEXT("Character should track whether ranged charge input is still held"), InputHeldProperty);
    TestNotNull(TEXT("Character should track whether ranged release was committed"), ReleaseCommittedProperty);
    TestNotNull(TEXT("Character should store the ranged target yaw"), FacingTargetYawProperty);
    TestNotNull(TEXT("Character should store ranged charge start time"), ChargeStartTimeProperty);
    TestNotNull(TEXT("Character should track ranged aim camera blend alpha"), CameraBlendAlphaProperty);
    TestNotNull(TEXT("Character should expose ranged minimum hold seconds"), MinimumHoldSecondsProperty);
    TestNotNull(TEXT("Character should track whether a ranged aim snapshot is cached"), CachedAimProperty);
    TestNotNull(TEXT("Character should cache the ranged aim origin used for projectile launch"), CachedAimOriginProperty);
    TestNotNull(TEXT("Character should cache the ranged aim target used for projectile launch"), CachedAimTargetProperty);
    TestNotNull(TEXT("Test should be able to assign a controller"), ControllerProperty);
    TestNotNull(TEXT("Test should be able to assign a player camera manager"), CameraManagerProperty);

    if (!MovementSuppressedProperty || !FacingLockedProperty || !InputHeldProperty || !ReleaseCommittedProperty || !FacingTargetYawProperty || !ChargeStartTimeProperty || !CameraBlendAlphaProperty || !MinimumHoldSecondsProperty || !CachedAimProperty || !CachedAimOriginProperty || !CachedAimTargetProperty || !ControllerProperty || !CameraManagerProperty)
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
    TestFalse(TEXT("Ranged aim snapshot should start uncached"), CachedAimProperty->GetPropertyValue_InContainer(Character));
    TestEqual(TEXT("Ranged aim camera blend should start disabled"), CameraBlendAlphaProperty->GetPropertyValue_InContainer(Character), 0.0f);
    TestEqual(TEXT("Ranged charge minimum hold should now be half a second"), MinimumHoldSecondsProperty->GetPropertyValue_InContainer(Character), 0.5f);

    Character->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
    TestFalse(TEXT("Ranged charge should be blocked while airborne"), Character->BeginRangedCharge(0.0f));
    TestFalse(TEXT("Airborne ranged block should not start the attack"), Character->IsRangedAttackInProgress());
    Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

    APlayerController* Controller = NewObject<APlayerController>(Character);
    TestNotNull(TEXT("Controller should be created for hold aim updates"), Controller);
    if (!Controller)
    {
        return false;
    }

    Controller->SetControlRotation(FRotator(0.0f, 0.0f, 0.0f));
    ControllerProperty->SetObjectPropertyValue_InContainer(Character, Controller);

    APlayerCameraManager* CameraManager = NewObject<APlayerCameraManager>(Controller);
    TestNotNull(TEXT("Player camera manager should be created for aim trace origin"), CameraManager);
    if (!CameraManager)
    {
        return false;
    }

    CameraManager->SetActorLocation(FVector(300.0f, 50.0f, 120.0f));
    CameraManager->SetActorRotation(FRotator(0.0f, 90.0f, 0.0f));
    CameraManagerProperty->SetObjectPropertyValue_InContainer(Controller, CameraManager);

    FVector AimPoint = FVector::ZeroVector;
    TestTrue(TEXT("Crosshair aim point should resolve from the player camera"), Character->ResolveRangedCrosshairAimPoint(AimPoint));
    const FVector ExpectedAimPoint =
        CameraManager->GetCameraLocation() + (CameraManager->GetCameraRotation().Vector() * Character->GetCombatComponent()->GetRangedAimTraceDistance());
    TestEqual(TEXT("Crosshair aim should originate from the active player camera"), AimPoint, ExpectedAimPoint);
    TestEqual(
        TEXT("Owning client notify should forward the cached aim to the server"),
        APvPArenaCharacter::ResolveRangedHitNotifyHandling(false, true, true),
        ERangedHitNotifyHandling::SendToServer);
    TestEqual(
        TEXT("Server host notify should trigger immediately"),
        APvPArenaCharacter::ResolveRangedHitNotifyHandling(true, true, true),
        ERangedHitNotifyHandling::TriggerImmediately);
    TestEqual(
        TEXT("Server should wait for the owning remote player notify RPC before firing"),
        APvPArenaCharacter::ResolveRangedHitNotifyHandling(true, false, true),
        ERangedHitNotifyHandling::Ignore);
    TestEqual(
        TEXT("Non-owning simulated proxy notify should stay ignored"),
        APvPArenaCharacter::ResolveRangedHitNotifyHandling(false, false, true),
        ERangedHitNotifyHandling::Ignore);

    Character->SetActorRotation(FRotator::ZeroRotator);
    TestTrue(TEXT("First ranged charge start should succeed"), Character->BeginRangedCharge(0.0f));
    TestTrue(TEXT("Ranged charge should enter in-progress state"), Character->IsRangedAttackInProgress());
    TestTrue(TEXT("Starting a ranged charge should track held input"), InputHeldProperty->GetPropertyValue_InContainer(Character));
    TestFalse(TEXT("Starting a ranged charge should not immediately commit release"), ReleaseCommittedProperty->GetPropertyValue_InContainer(Character));
    TestFalse(TEXT("Starting a ranged charge should not immediately trigger the hit"), Character->HasTriggeredRangedAttackHit());
    TestEqual(TEXT("Ranged charge should record its start time"), ChargeStartTimeProperty->GetPropertyValue_InContainer(Character), 0.0f);
    TestTrue(TEXT("Starting a ranged charge should suppress movement"), MovementSuppressedProperty->GetPropertyValue_InContainer(Character));
    TestTrue(TEXT("Starting a ranged charge should lock facing"), FacingLockedProperty->GetPropertyValue_InContainer(Character));

    Character->AdvanceAttackFacing(0.1f);
    TestTrue(TEXT("Holding ranged charge should push aim camera blend up"), CameraBlendAlphaProperty->GetPropertyValue_InContainer(Character) > 0.0f);

    Character->AdvanceAttackFacing(0.1f);

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
    TestTrue(TEXT("Release should cache the current aim snapshot immediately"), CachedAimProperty->GetPropertyValue_InContainer(Character));
    const FVector* ReleaseCachedAimOrigin = CachedAimOriginProperty->ContainerPtrToValuePtr<FVector>(Character);
    const FVector* ReleaseCachedAimTarget = CachedAimTargetProperty->ContainerPtrToValuePtr<FVector>(Character);
    TestNotNull(TEXT("Release cached ranged aim origin should be readable"), ReleaseCachedAimOrigin);
    TestNotNull(TEXT("Release cached ranged aim target should be readable"), ReleaseCachedAimTarget);
    const FVector CachedAimOriginAtRelease = ReleaseCachedAimOrigin ? *ReleaseCachedAimOrigin : FVector::ZeroVector;
    const FVector CachedAimAtRelease = ReleaseCachedAimTarget ? *ReleaseCachedAimTarget : FVector::ZeroVector;

    UFunction* ServerHandleRangedAttackHitNotifyFunction = Character->FindFunction(TEXT("ServerHandleRangedAttackHitNotify"));
    TestNotNull(TEXT("Server ranged hit notify RPC should exist"), ServerHandleRangedAttackHitNotifyFunction);
    if (!ServerHandleRangedAttackHitNotifyFunction)
    {
        return false;
    }

    FServerHandleRangedAttackHitNotifyParams ServerNotifyParams;
    Character->ProcessEvent(ServerHandleRangedAttackHitNotifyFunction, &ServerNotifyParams);
    const FVector* CachedAimOriginAfterServerNotify = CachedAimOriginProperty->ContainerPtrToValuePtr<FVector>(Character);
    const FVector* CachedAimTargetAfterServerNotify = CachedAimTargetProperty->ContainerPtrToValuePtr<FVector>(Character);
    TestEqual(
        TEXT("Server ranged hit notify without client aim should preserve the release-time aim origin"),
        CachedAimOriginAfterServerNotify ? *CachedAimOriginAfterServerNotify : FVector::ZeroVector,
        CachedAimOriginAtRelease);
    TestEqual(
        TEXT("Server ranged hit notify without client aim should preserve the release-time aim target"),
        CachedAimTargetAfterServerNotify ? *CachedAimTargetAfterServerNotify : FVector::ZeroVector,
        CachedAimAtRelease);

    ServerNotifyParams.bHasAim = true;
    ServerNotifyParams.AimOrigin = FVector(999.0f, 888.0f, 77.0f);
    ServerNotifyParams.AimTarget = FVector(-555.0f, 444.0f, 33.0f);
    Character->ProcessEvent(ServerHandleRangedAttackHitNotifyFunction, &ServerNotifyParams);
    CachedAimOriginAfterServerNotify = CachedAimOriginProperty->ContainerPtrToValuePtr<FVector>(Character);
    CachedAimTargetAfterServerNotify = CachedAimTargetProperty->ContainerPtrToValuePtr<FVector>(Character);
    TestEqual(
        TEXT("Server ranged hit notify with late client aim should preserve the release-time aim origin"),
        CachedAimOriginAfterServerNotify ? *CachedAimOriginAfterServerNotify : FVector::ZeroVector,
        CachedAimOriginAtRelease);
    TestEqual(
        TEXT("Server ranged hit notify with late client aim should preserve the release-time aim target"),
        CachedAimTargetAfterServerNotify ? *CachedAimTargetAfterServerNotify : FVector::ZeroVector,
        CachedAimAtRelease);
    Character->FinishRangedAttack();
    TestFalse(TEXT("Reset after direct server notify should clear ranged state"), Character->IsRangedAttackInProgress());

    TestTrue(TEXT("A fresh ranged charge should start for local notify checks"), Character->BeginRangedCharge(10.0f));
    TestTrue(TEXT("A fresh ranged release should commit for local notify checks"), Character->ReleaseRangedCharge(11.0f));
    Controller->SetControlRotation(FRotator(0.0f, 160.0f, 0.0f));
    CameraManager->SetActorRotation(FRotator(0.0f, 160.0f, 0.0f));
    Character->HandleRangedAttackHitNotify();
    TestTrue(TEXT("Ranged hit should be marked triggered after notify"), Character->HasTriggeredRangedAttackHit());
    const FVector* CachedAimTarget = CachedAimTargetProperty->ContainerPtrToValuePtr<FVector>(Character);
    TestNotNull(TEXT("Cached ranged aim target should be readable"), CachedAimTarget);
    TestEqual(TEXT("Ranged notify should keep the release-time aim snapshot"), CachedAimTarget ? *CachedAimTarget : FVector::ZeroVector, CachedAimAtRelease);
    TestFalse(TEXT("Ranged notify should not trigger twice"), Character->HandleRangedAttackHitNotify());

    Character->FinishRangedAttack();
    TestFalse(TEXT("Finish should clear ranged in-progress state"), Character->IsRangedAttackInProgress());
    TestFalse(TEXT("Finish should clear ranged hit-triggered state"), Character->HasTriggeredRangedAttackHit());
    TestFalse(TEXT("Finish should clear cached ranged aim snapshot"), CachedAimProperty->GetPropertyValue_InContainer(Character));
    TestFalse(TEXT("Finish should clear movement suppression"), MovementSuppressedProperty->GetPropertyValue_InContainer(Character));
    TestFalse(TEXT("Finish should clear facing lock"), FacingLockedProperty->GetPropertyValue_InContainer(Character));

    return true;
}
