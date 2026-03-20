#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PvPProjectile.generated.h"

class APvPArenaCharacter;
class UNiagaraComponent;
class UNiagaraSystem;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class PVPARENA_API APvPProjectile : public AActor
{
    GENERATED_BODY()

public:
    APvPProjectile();

    void InitializeProjectile(APvPArenaCharacter* InInstigatorCharacter, float InDamage, const FVector& InLaunchDirection);

protected:
    virtual void Tick(float DeltaSeconds) override;
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void HandleProjectileHit(
        UPrimitiveComponent* HitComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        FVector NormalImpulse,
        const FHitResult& Hit);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayImpactEffect(FVector_NetQuantize ImpactLocation, FVector_NetQuantizeNormal ImpactNormal);

    UPROPERTY(VisibleAnywhere, Category = "Projectile")
    TObjectPtr<USphereComponent> CollisionComponent;

    UPROPERTY(VisibleAnywhere, Category = "Projectile")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

    UPROPERTY(VisibleAnywhere, Category = "Projectile|Visual")
    TObjectPtr<UStaticMeshComponent> VisualMeshComponent;

    UPROPERTY(VisibleAnywhere, Category = "Projectile|Visual")
    TObjectPtr<UNiagaraComponent> ProjectileEffectComponent;

    UPROPERTY()
    TObjectPtr<APvPArenaCharacter> InstigatorCharacter;

    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    float InitialLifeSeconds = 3.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Projectile|Visual")
    TObjectPtr<UNiagaraSystem> ImpactEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Debug")
    bool bDrawProjectileDebug = false;

    UPROPERTY(EditDefaultsOnly, Category = "Debug")
    float ProjectileDebugDrawTime = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Debug")
    float ProjectileDebugLineThickness = 1.5f;

    FVector PreviousDebugDrawLocation = FVector::ZeroVector;
    FVector LaunchDirection = FVector::ForwardVector;
    float Damage = 0.0f;
};
