#include "Player/PvPArenaPlayerController.h"

#include "Player/PvPArenaCharacter.h"

void APvPArenaPlayerController::ServerRequestRespawn_Implementation()
{
    APvPArenaCharacter* PvPCharacter = Cast<APvPArenaCharacter>(GetPawn());
    if (!PvPCharacter)
    {
        return;
    }

    if (PvPCharacter->CanRespawnAtTime(GetWorld()->GetTimeSeconds()))
    {
        PvPCharacter->RespawnToFullHealth();
    }
}
