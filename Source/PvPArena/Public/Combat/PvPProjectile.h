#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PvPProjectile.generated.h"

class APvPArenaCharacter;
class UNiagaraComponent;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class PVPARENA_API APvPProjectile : public AActor
{
    GENERATED_BODY()

public:
    APvPProjectile();

    void InitializeProjectile(APvPArenaCharacter* InInstigatorCharacter, float InDamage);

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void HandleProjectileHit(
        UPrimitiveComponent* HitComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        FVector NormalImpulse,
        const FHitResult& Hit);

    UPROPERTY(VisibleAnywhere, Category = "Projectile")
    TObjectPtr<USphereComponent> CollisionComponent;

    UPROPERTY(VisibleAnywhere, Category = "Projectile")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

    UPROPERTY(VisibleAnywhere, Category = "Projectile")
    TObjectPtr<UNiagaraComponent> ProjectileEffectComponent;

    UPROPERTY()
    TObjectPtr<APvPArenaCharacter> InstigatorCharacter;

    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    float InitialLifeSeconds = 3.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    FVector ProjectileEffectRelativeOffset = FVector(0.0f, 0.0f, 18.0f);

    float Damage = 0.0f;
};
