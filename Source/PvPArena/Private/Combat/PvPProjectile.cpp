#include "Combat/PvPProjectile.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Player/PvPArenaCharacter.h"
#include "UObject/ConstructorHelpers.h"

APvPProjectile::APvPProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    SetActorTickEnabled(false);
    bReplicates = true;
    SetReplicateMovement(true);

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
    ProjectileMovementComponent->InitialSpeed = 3600.0f;
    ProjectileMovementComponent->MaxSpeed = 3600.0f;
    ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->bInitialVelocityInLocalSpace = false;
    ProjectileMovementComponent->bShouldBounce = false;

    VisualMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMeshComponent"));
    VisualMeshComponent->SetupAttachment(CollisionComponent);
    VisualMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VisualMeshComponent->SetRelativeLocation(FVector::ZeroVector);
    VisualMeshComponent->SetRelativeRotation(FRotator::ZeroRotator);
    VisualMeshComponent->SetRelativeScale3D(FVector(0.45f));

    ProjectileEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileEffectComponent"));
    ProjectileEffectComponent->SetupAttachment(CollisionComponent);
    ProjectileEffectComponent->SetAutoActivate(false);
    ProjectileEffectComponent->SetUsingAbsoluteScale(false);
    ProjectileEffectComponent->SetRelativeLocation(FVector::ZeroVector);
    ProjectileEffectComponent->SetRelativeRotation(FRotator::ZeroRotator);
    ProjectileEffectComponent->SetRelativeScale3D(FVector(3.0f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> VisualMeshFinder(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (VisualMeshFinder.Succeeded())
    {
        VisualMeshComponent->SetStaticMesh(VisualMeshFinder.Object);
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> VisualMaterialFinder(
        TEXT("/Game/PvPArena/VFX/Crystal/Materials/Instance_Materials/MI_Colorful_Inst_10.MI_Colorful_Inst_10"));
    if (VisualMaterialFinder.Succeeded())
    {
        VisualMeshComponent->SetMaterial(0, VisualMaterialFinder.Object);
    }

    static ConstructorHelpers::FObjectFinder<UNiagaraSystem> ProjectileEffectFinder(
        TEXT("/Game/PvPArena/VFX/Crystal/VFX/NS_Crystal_Torrent.NS_Crystal_Torrent"));
    if (ProjectileEffectFinder.Succeeded())
    {
        ProjectileEffectComponent->SetAsset(ProjectileEffectFinder.Object);
    }

    static ConstructorHelpers::FObjectFinder<UNiagaraSystem> ImpactEffectFinder(
        TEXT("/Game/PvPArena/VFX/Crystal/VFX/NS_Crystal_Torrent_Owner_Cast.NS_Crystal_Torrent_Owner_Cast"));
    if (ImpactEffectFinder.Succeeded())
    {
        ImpactEffect = ImpactEffectFinder.Object;
    }
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
    PreviousDebugDrawLocation = bDrawProjectileDebug ? GetActorLocation() : FVector::ZeroVector;

    if (CollisionComponent && InInstigatorCharacter)
    {
        CollisionComponent->IgnoreActorWhenMoving(InInstigatorCharacter, true);
    }
}

void APvPProjectile::BeginPlay()
{
    Super::BeginPlay();

    SetLifeSpan(InitialLifeSeconds);
    SetActorTickEnabled(bDrawProjectileDebug);
    PreviousDebugDrawLocation = bDrawProjectileDebug ? GetActorLocation() : FVector::ZeroVector;

    if (CollisionComponent)
    {
        CollisionComponent->OnComponentHit.AddDynamic(this, &APvPProjectile::HandleProjectileHit);
    }

    if (!HasAuthority())
    {
        if (CollisionComponent)
        {
            CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        if (ProjectileMovementComponent)
        {
            ProjectileMovementComponent->StopMovementImmediately();
            ProjectileMovementComponent->Deactivate();
        }

        return;
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

    MulticastPlayImpactEffect(Hit.ImpactPoint, Hit.ImpactNormal);

    Destroy();
}

void APvPProjectile::MulticastPlayImpactEffect_Implementation(FVector_NetQuantize ImpactLocation, FVector_NetQuantizeNormal ImpactNormal)
{
    if (!ImpactEffect)
    {
        return;
    }

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        ImpactEffect,
        ImpactLocation,
        ImpactNormal.Rotation());
}
