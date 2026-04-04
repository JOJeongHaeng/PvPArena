#include "Player/PvPArenaSpectatorPawn.h"

APvPArenaSpectatorPawn::APvPArenaSpectatorPawn()
{
    PrimaryActorTick.bCanEverTick = false;
    bAddDefaultMovementBindings = false;
    SetActorEnableCollision(false);
}
