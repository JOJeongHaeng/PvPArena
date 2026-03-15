#include "Animation/PvPAnimNotify_MeleeHit.h"

#include "Components/SkeletalMeshComponent.h"
#include "Player/PvPArenaCharacter.h"

void UPvPAnimNotify_MeleeHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    APvPArenaCharacter* Character = MeshComp ? Cast<APvPArenaCharacter>(MeshComp->GetOwner()) : nullptr;
    if (!Character)
    {
        return;
    }

    Character->HandleMeleeAttackHitNotify();
}
