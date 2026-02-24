#include "Player/PvPArenaCharacter.h"

#include "Net/UnrealNetwork.h"

APvPArenaCharacter::APvPArenaCharacter()
{
    bReplicates = true;
}

void APvPArenaCharacter::OnRep_CurrentHealth()
{
    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
}

void APvPArenaCharacter::OnRep_IsDead()
{
}

void APvPArenaCharacter::ApplyServerDamage(float Damage, AController* InstigatorController)
{
    (void)InstigatorController;

    if (bIsDead || Damage <= 0.0f)
    {
        return;
    }

    CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
    if (CurrentHealth <= 0.0f)
    {
        bIsDead = true;
    }
}

void APvPArenaCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APvPArenaCharacter, CurrentHealth);
    DOREPLIFETIME(APvPArenaCharacter, bIsDead);
}
