#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PvPArenaWeaponComponent.generated.h"

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class PVPARENA_API UPvPArenaWeaponComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPvPArenaWeaponComponent();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(Server, Reliable)
    void ServerTryFire(FVector_NetQuantize TraceStart, FVector_NetQuantizeNormal Dir, uint8 ShotSeq);

    UFUNCTION(Server, Reliable)
    void ServerReload();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    int32 MaxAmmo;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
    int32 CurrentAmmo;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
    bool bIsReloading;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    float FireIntervalSeconds;

private:
    bool CanServerFire() const;

    float LastServerFireTimeSeconds;
};
