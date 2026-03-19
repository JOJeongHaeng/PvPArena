#include "Combat/PvPCombatComponent.h"

#include "Combat/PvPProjectile.h"
#include "Components/SkeletalMeshComponent.h"
#include "Player/PvPArenaCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Net/UnrealNetwork.h"

UPvPCombatComponent::UPvPCombatComponent()
{
    SetIsReplicatedByDefault(true);
    RangedProjectileClass = APvPProjectile::StaticClass();
}

bool UPvPCombatComponent::CanUseMelee(float NowSeconds) const
{
    return NowSeconds >= NextAllowedMeleeTime;
}

float UPvPCombatComponent::GetRemainingRangedCooldown(float NowSeconds) const
{
    return FMath::Max(0.0f, NextAllowedRangedTime - NowSeconds);
}

float UPvPCombatComponent::GetRangedCooldownAlpha(float NowSeconds) const
{
    if (RangedCooldownSeconds <= 0.0f)
    {
        return 1.0f;
    }

    return 1.0f - (GetRemainingRangedCooldown(NowSeconds) / RangedCooldownSeconds);
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

    FVector Start = Attacker->GetActorLocation();
    if (USkeletalMeshComponent* Mesh = Attacker->GetMesh())
    {
        const FName SocketName = Attacker->GetMeleeAttackSocketName();
        if (Mesh->DoesSocketExist(SocketName))
        {
            Start = Mesh->GetSocketLocation(SocketName);
        }
    }

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
        Attacker->ShowMeleeDebug(Start, End, bHit);
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
    if (!Attacker || !Attacker->GetWorld() || !RangedProjectileClass)
    {
        return false;
    }

    FVector ViewLocation;
    FRotator ViewRotation;
    if (AController* Controller = Attacker->GetController())
    {
        Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
        if (Attacker->IsRangedAttackInProgress())
        {
            ViewRotation.Yaw = Attacker->GetRangedAttackTargetYaw();
        }
    }
    else
    {
        ViewLocation = Attacker->GetActorLocation();
        ViewRotation = Attacker->GetActorRotation();
    }

    const FVector AimStart = ViewLocation;
    const FVector AimDirection = ViewRotation.Vector();
    const FVector AimEnd = AimStart + (AimDirection * RangedRange);

    FCollisionQueryParams AimQueryParams(SCENE_QUERY_STAT(PvPArenaRangedAimTrace), false, Attacker);
    AimQueryParams.AddIgnoredActor(Attacker);

    FHitResult AimHitResult;
    Attacker->GetWorld()->LineTraceSingleByChannel(
        AimHitResult,
        AimStart,
        AimEnd,
        ECC_Visibility,
        AimQueryParams);

    const FVector AimTarget = AimHitResult.bBlockingHit ? AimHitResult.ImpactPoint : AimEnd;
    const FVector SpawnLocation = Attacker->GetActorLocation()
        + (Attacker->GetActorForwardVector() * RangedSpawnForwardOffset)
        + (FVector::UpVector * RangedSpawnHeightOffset);
    const FVector Direction = (AimTarget - SpawnLocation).GetSafeNormal();
    const FTransform SpawnTransform(Direction.Rotation(), SpawnLocation);
    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = Attacker;
    SpawnParameters.Instigator = Attacker;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    APvPProjectile* Projectile = Attacker->GetWorld()->SpawnActorDeferred<APvPProjectile>(
        RangedProjectileClass,
        SpawnTransform,
        Attacker,
        Attacker,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

    if (!Projectile)
    {
        return false;
    }

    Projectile->InitializeProjectile(Attacker, RangedDamage);
    Projectile->FinishSpawning(SpawnTransform);

    if (bDrawAttackDebug)
    {
        DrawDebugLine(Attacker->GetWorld(), SpawnLocation, AimTarget, FColor::Cyan, false, DebugDrawTime, 0, 1.5f);
        DrawDebugPoint(Attacker->GetWorld(), SpawnLocation, 10.0f, FColor::Cyan, false, DebugDrawTime, 0);
        DrawDebugPoint(Attacker->GetWorld(), AimTarget, 12.0f, AimHitResult.bBlockingHit ? FColor::Red : FColor::Green, false, DebugDrawTime, 0);
    }

    return true;
}

void UPvPCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UPvPCombatComponent, NextAllowedRangedTime);
}
