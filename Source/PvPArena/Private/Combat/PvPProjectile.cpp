#include "Combat/PvPProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Player/PvPArenaCharacter.h"
#include "UObject/ConstructorHelpers.h"

APvPProjectile::APvPProjectile()
{
    PrimaryActorTick.bCanEverTick = false;
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
    ProjectileMovementComponent->InitialSpeed = 2200.0f;
    ProjectileMovementComponent->MaxSpeed = 2200.0f;
    ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->bShouldBounce = false;

    ProjectileEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileEffectComponent"));
    ProjectileEffectComponent->SetupAttachment(CollisionComponent);
    ProjectileEffectComponent->SetAutoActivate(true);
    ProjectileEffectComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ProjectileEffectComponent->SetRelativeLocation(ProjectileEffectRelativeOffset);

    static ConstructorHelpers::FObjectFinder<UNiagaraSystem> ProjectileEffectFinder(
        TEXT("/Game/PvPArena/VFX/Mixed_Magic_VFX_Pack/VFX/NS_Magma_Shot.NS_Magma_Shot"));
    if (ProjectileEffectFinder.Succeeded())
    {
        ProjectileEffectComponent->SetAsset(ProjectileEffectFinder.Object);
    }
}

void APvPProjectile::InitializeProjectile(APvPArenaCharacter* InInstigatorCharacter, float InDamage)
{
    InstigatorCharacter = InInstigatorCharacter;
    Damage = InDamage;
    SetOwner(InInstigatorCharacter);
    SetInstigator(InInstigatorCharacter);

    if (CollisionComponent && InInstigatorCharacter)
    {
        CollisionComponent->IgnoreActorWhenMoving(InInstigatorCharacter, true);
    }
}

void APvPProjectile::BeginPlay()
{
    Super::BeginPlay();

    SetLifeSpan(InitialLifeSeconds);

    if (CollisionComponent)
    {
        CollisionComponent->OnComponentHit.AddDynamic(this, &APvPProjectile::HandleProjectileHit);
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
