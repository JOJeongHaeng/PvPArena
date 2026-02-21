#include "Combat/PvPArenaWeaponComponent.h"

#include "Net/UnrealNetwork.h"
#include "Player/PvPArenaCharacter.h"

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

    (void)TraceStart;
    (void)Dir;
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
