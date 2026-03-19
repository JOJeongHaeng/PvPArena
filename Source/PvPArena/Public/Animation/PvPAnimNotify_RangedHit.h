#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"
#include "PvPAnimNotify_RangedHit.generated.h"

UCLASS()
class PVPARENA_API UPvPAnimNotify_RangedHit : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
