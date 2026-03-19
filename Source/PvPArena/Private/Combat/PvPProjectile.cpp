#include "Combat/PvPProjectile.h"

#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Player/PvPArenaCharacter.h"

APvPProjectile::APvPProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    SetRootComponent(CollisionComponent);
    CollisionComponent->InitSphereRadius(16.0f);
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    CollisionComponent->SetNotifyRigidBodyCollision(true);

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->InitialSpeed = 1800.0f;
    ProjectileMovementComponent->MaxSpeed = 1800.0f;
    ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->bInitialVelocityInLocalSpace = false;
    ProjectileMovementComponent->bShouldBounce = false;

}

void APvPProjectile::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bDrawProjectileDebug)
    {
        return;
    }

    if (PreviousDebugDrawLocation.IsNearlyZero())
    {
        PreviousDebugDrawLocation = GetActorLocation();
        return;
    }

    DrawDebugLine(
        GetWorld(),
        PreviousDebugDrawLocation,
        GetActorLocation(),
        FColor::Orange,
        false,
        ProjectileDebugDrawTime,
        0,
        ProjectileDebugLineThickness);
    PreviousDebugDrawLocation = GetActorLocation();
}

void APvPProjectile::InitializeProjectile(APvPArenaCharacter* InInstigatorCharacter, float InDamage, const FVector& InLaunchDirection)
{
    InstigatorCharacter = InInstigatorCharacter;
    Damage = InDamage;
    LaunchDirection = InLaunchDirection.GetSafeNormal();
    SetOwner(InInstigatorCharacter);
    SetInstigator(InInstigatorCharacter);
    SetActorRotation(LaunchDirection.Rotation());
    PreviousDebugDrawLocation = GetActorLocation();

    if (CollisionComponent && InInstigatorCharacter)
    {
        CollisionComponent->IgnoreActorWhenMoving(InInstigatorCharacter, true);
    }
}

void APvPProjectile::BeginPlay()
{
    Super::BeginPlay();

    SetLifeSpan(InitialLifeSeconds);
    PreviousDebugDrawLocation = GetActorLocation();

    if (CollisionComponent)
    {
        CollisionComponent->OnComponentHit.AddDynamic(this, &APvPProjectile::HandleProjectileHit);
    }

    if (ProjectileMovementComponent)
    {
        ProjectileMovementComponent->Velocity = LaunchDirection * ProjectileMovementComponent->InitialSpeed;
        ProjectileMovementComponent->UpdateComponentVelocity();
    }
}

void APvPProjectile::HandleProjectileHit(
    UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    FVector NormalImpulse,
    const FHitResult& Hit)
{
    if (!HasAuthority())
    {
        return;
    }

    APvPArenaCharacter* HitCharacter = Cast<APvPArenaCharacter>(OtherActor);
    if (HitCharacter && HitCharacter != InstigatorCharacter && !HitCharacter->IsDead())
    {
        HitCharacter->ApplyServerDamage(Damage, InstigatorCharacter ? InstigatorCharacter->GetController() : nullptr);
    }

    Destroy();
}
