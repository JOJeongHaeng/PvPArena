#include "Combat/PvPArenaWeaponComponent.h"

#include "Game/PvPArenaGameState.h"
#include "Net/UnrealNetwork.h"
#include "Player/PvPArenaCharacter.h"
#include "Player/PvPArenaPlayerState.h"

UPvPArenaWeaponComponent::UPvPArenaWeaponComponent()
{
    SetIsReplicatedByDefault(true);

    MaxAmmo = 30;
    CurrentAmmo = MaxAmmo;
    bIsReloading = false;
    FireIntervalSeconds = 0.1f;
    LastServerFireTimeSeconds = -1000.0f;
}

void UPvPArenaWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UPvPArenaWeaponComponent, CurrentAmmo);
    DOREPLIFETIME(UPvPArenaWeaponComponent, bIsReloading);
}

void UPvPArenaWeaponComponent::ServerTryFire_Implementation(FVector_NetQuantize TraceStart, FVector_NetQuantizeNormal Dir, uint8 ShotSeq)
{
    if (!CanServerFire())
    {
        return;
    }

    const UWorld* World = GetWorld();
    LastServerFireTimeSeconds = World ? World->GetTimeSeconds() : LastServerFireTimeSeconds;
    --CurrentAmmo;

    bIsReloading = false;

    FHitResult HitResult;
    if (World)
    {
        const FVector TraceEnd = TraceStart + (FVector(Dir) * 10000.0f);
        FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ServerWeaponTrace), false, GetOwner());
        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
        {
            if (APvPArenaCharacter* HitCharacter = Cast<APvPArenaCharacter>(HitResult.GetActor()))
            {
                HandleConfirmedHit(HitCharacter, 34.0f);
            }
        }
    }

    (void)ShotSeq;
}

void UPvPArenaWeaponComponent::ServerReload_Implementation()
{
    APvPArenaCharacter* OwnerCharacter = Cast<APvPArenaCharacter>(GetOwner());
    if (!OwnerCharacter || OwnerCharacter->bIsDead || CurrentAmmo >= MaxAmmo)
    {
        return;
    }

    bIsReloading = true;
    CurrentAmmo = MaxAmmo;
    bIsReloading = false;
}

bool UPvPArenaWeaponComponent::CanServerFire() const
{
    const APvPArenaCharacter* OwnerCharacter = Cast<APvPArenaCharacter>(GetOwner());
    if (!OwnerCharacter || OwnerCharacter->bIsDead || bIsReloading || CurrentAmmo <= 0)
    {
        return false;
    }

    const UWorld* World = GetWorld();
    const float Now = World ? World->GetTimeSeconds() : 0.0f;
    return (Now - LastServerFireTimeSeconds) >= FireIntervalSeconds;
}

void UPvPArenaWeaponComponent::HandleConfirmedHit(APvPArenaCharacter* VictimCharacter, float DamageAmount)
{
    APvPArenaCharacter* InstigatorCharacter = Cast<APvPArenaCharacter>(GetOwner());
    if (!InstigatorCharacter || !VictimCharacter || InstigatorCharacter == VictimCharacter || DamageAmount <= 0.0f)
    {
        return;
    }

    const bool bVictimWasDead = VictimCharacter->bIsDead;
    VictimCharacter->ApplyDamageServer(DamageAmount);

    if (bVictimWasDead || !VictimCharacter->bIsDead)
    {
        return;
    }

    APvPArenaPlayerState* InstigatorState = InstigatorCharacter->GetPlayerState<APvPArenaPlayerState>();
    if (InstigatorState)
    {
        InstigatorState->AddKill();
    }

    if (APvPArenaGameState* PvPGameState = GetWorld() ? GetWorld()->GetGameState<APvPArenaGameState>() : nullptr)
    {
        PvPGameState->AddTeamScore(InstigatorState ? InstigatorState->TeamId : 0, 1);
    }
}
