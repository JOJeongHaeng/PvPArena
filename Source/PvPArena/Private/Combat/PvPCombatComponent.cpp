#include "Combat/PvPCombatComponent.h"

#include "Player/PvPArenaCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"

bool UPvPCombatComponent::CanUseMelee(float NowSeconds) const
{
    return NowSeconds >= NextAllowedMeleeTime;
}

bool UPvPCombatComponent::CanUseRanged(float NowSeconds) const
{
    return NowSeconds >= NextAllowedRangedTime;
}

void UPvPCombatComponent::MarkMeleeUsed(float NowSeconds)
{
    NextAllowedMeleeTime = NowSeconds + MeleeCooldownSeconds;
}

void UPvPCombatComponent::MarkRangedUsed(float NowSeconds)
{
    NextAllowedRangedTime = NowSeconds + RangedCooldownSeconds;
}

bool UPvPCombatComponent::TryServerMeleeAttack(APvPArenaCharacter* Attacker)
{
    if (!Attacker || !Attacker->GetWorld())
    {
        return false;
    }

    const FVector Start = Attacker->GetActorLocation();
    const FVector End = Start + (Attacker->GetActorForwardVector() * MeleeRange);

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PvPArenaMeleeTrace), false, Attacker);
    FCollisionObjectQueryParams ObjectQueryParams;
    ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
    FHitResult HitResult;
    const bool bHit = Attacker->GetWorld()->SweepSingleByObjectType(
        HitResult,
        Start,
        End,
        FQuat::Identity,
        ObjectQueryParams,
        FCollisionShape::MakeSphere(MeleeRadius),
        QueryParams);

    if (bDrawAttackDebug)
    {
        const FColor DebugColor = bHit ? FColor::Red : FColor::Green;
        DrawDebugCapsule(
            Attacker->GetWorld(),
            (Start + End) * 0.5f,
            MeleeRange * 0.5f,
            MeleeRadius,
            FRotationMatrix::MakeFromZ((End - Start).GetSafeNormal()).ToQuat(),
            DebugColor,
            false,
            DebugDrawTime,
            0,
            1.5f);
        DrawDebugSphere(Attacker->GetWorld(), End, MeleeRadius, 16, DebugColor, false, DebugDrawTime, 0, 1.0f);
    }

    APvPArenaCharacter* HitCharacter = bHit ? Cast<APvPArenaCharacter>(HitResult.GetActor()) : nullptr;
    if (!HitCharacter || HitCharacter == Attacker || HitCharacter->IsDead())
    {
        return false;
    }

    HitCharacter->ApplyServerDamage(MeleeDamage, Attacker->GetController());
    return true;
}

bool UPvPCombatComponent::TryServerRangedAttack(APvPArenaCharacter* Attacker)
{
    if (!Attacker || !Attacker->GetWorld())
    {
        return false;
    }

    FVector ViewLocation;
    FRotator ViewRotation;
    if (AController* Controller = Attacker->GetController())
    {
        Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
    }
    else
    {
        ViewLocation = Attacker->GetActorLocation();
        ViewRotation = Attacker->GetActorRotation();
    }

    const FVector Start = ViewLocation;
    const FVector End = Start + (ViewRotation.Vector() * RangedRange);
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PvPArenaRangedTrace), false, Attacker);
    FCollisionObjectQueryParams ObjectQueryParams;
    ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

    FHitResult HitResult;
    const bool bHit = Attacker->GetWorld()->LineTraceSingleByObjectType(
        HitResult,
        Start,
        End,
        ObjectQueryParams,
        QueryParams);

    if (bDrawAttackDebug)
    {
        const FVector DebugEnd = bHit ? HitResult.ImpactPoint : End;
        const FColor DebugColor = bHit ? FColor::Red : FColor::Green;
        DrawDebugLine(Attacker->GetWorld(), Start, DebugEnd, DebugColor, false, DebugDrawTime, 0, 1.5f);
        DrawDebugPoint(Attacker->GetWorld(), DebugEnd, 10.0f, DebugColor, false, DebugDrawTime, 0);
    }

    APvPArenaCharacter* HitCharacter = bHit ? Cast<APvPArenaCharacter>(HitResult.GetActor()) : nullptr;
    if (!HitCharacter || HitCharacter == Attacker || HitCharacter->IsDead())
    {
        return false;
    }

    HitCharacter->ApplyServerDamage(RangedDamage, Attacker->GetController());
    return true;
}
