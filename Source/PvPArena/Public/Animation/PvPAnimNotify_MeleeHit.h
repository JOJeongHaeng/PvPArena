#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"
#include "PvPAnimNotify_MeleeHit.generated.h"

UCLASS()
class PVPARENA_API UPvPAnimNotify_MeleeHit : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
