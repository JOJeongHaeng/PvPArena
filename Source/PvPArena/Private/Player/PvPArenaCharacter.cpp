#include "Player/PvPArenaCharacter.h"

#include "Net/UnrealNetwork.h"
#include "Player/PvPArenaPlayerState.h"

APvPArenaCharacter::APvPArenaCharacter()
{
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    bIsDead = false;
    RespawnDelaySeconds = 5.0f;
    DeathServerTimeSeconds = -1.0f;
}

void APvPArenaCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APvPArenaCharacter, CurrentHealth);
    DOREPLIFETIME(APvPArenaCharacter, bIsDead);
}

void APvPArenaCharacter::ApplyDamageServer(float DamageAmount)
{
    if (!HasAuthority() || bIsDead || DamageAmount <= 0.0f)
    {
        return;
    }

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);
    if (CurrentHealth <= 0.0f)
    {
        EnterDeadState();
    }
}

bool APvPArenaCharacter::CanRespawnAtTime(float ServerTimeSeconds) const
{
    if (!bIsDead)
    {
        return false;
    }

    return DeathServerTimeSeconds >= 0.0f && (ServerTimeSeconds - DeathServerTimeSeconds) >= RespawnDelaySeconds;
}

void APvPArenaCharacter::RespawnToFullHealth()
{
    if (!HasAuthority())
    {
        return;
    }

    bIsDead = false;
    CurrentHealth = MaxHealth;
    DeathServerTimeSeconds = -1.0f;

    if (APvPArenaPlayerState* PvPPlayerState = GetPlayerState<APvPArenaPlayerState>())
    {
        PvPPlayerState->SetDeadState(false, 0);
    }
}

void APvPArenaCharacter::EnterDeadState()
{
    bIsDead = true;
    DeathServerTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    if (APvPArenaPlayerState* PvPPlayerState = GetPlayerState<APvPArenaPlayerState>())
    {
        PvPPlayerState->AddDeath();
        PvPPlayerState->SetDeadState(true, FMath::RoundToInt(RespawnDelaySeconds));
    }
}
