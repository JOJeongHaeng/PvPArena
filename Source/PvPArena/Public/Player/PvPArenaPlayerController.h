#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PvPArenaPlayerController.generated.h"

UCLASS()
class PVPARENA_API APvPArenaPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    UFUNCTION(Server, Reliable)
    void ServerRequestRespawn();

protected:
    void ServerRequestRespawn_Implementation();
};
