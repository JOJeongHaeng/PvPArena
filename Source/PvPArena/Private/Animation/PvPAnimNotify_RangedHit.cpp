#include "Animation/PvPAnimNotify_RangedHit.h"

#include "Components/SkeletalMeshComponent.h"
#include "Player/PvPArenaCharacter.h"

void UPvPAnimNotify_RangedHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    APvPArenaCharacter* Character = MeshComp ? Cast<APvPArenaCharacter>(MeshComp->GetOwner()) : nullptr;
    if (!Character)
    {
        return;
    }

    Character->HandleRangedAttackHitNotify();
}
