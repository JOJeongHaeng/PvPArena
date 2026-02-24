#include "Combat/PvPCombatComponent.h"

#include "Player/PvPArenaCharacter.h"
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
    FHitResult HitResult;
    const bool bHit = Attacker->GetWorld()->SweepSingleByChannel(
        HitResult,
        Start,
        End,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(MeleeRadius),
        QueryParams);

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

    FHitResult HitResult;
    const bool bHit = Attacker->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Pawn,
        QueryParams);

    APvPArenaCharacter* HitCharacter = bHit ? Cast<APvPArenaCharacter>(HitResult.GetActor()) : nullptr;
    if (!HitCharacter || HitCharacter == Attacker || HitCharacter->IsDead())
    {
        return false;
    }

    HitCharacter->ApplyServerDamage(RangedDamage, Attacker->GetController());
    return true;
}
