#include "UI/PvPArenaHUD.h"

#include "Engine/Engine.h"
#include "Game/PvPArenaGameState.h"
#include "Game/PvPArenaPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Player/PvPArenaCharacter.h"

void APvPArenaHUD::DrawHUD()
{
    Super::DrawHUD();

    APlayerController* PC = GetOwningPlayerController();
    if (!PC)
    {
        return;
    }

    float Y = 40.0f;

    if (const APvPArenaCharacter* Character = Cast<APvPArenaCharacter>(PC->GetPawn()))
    {
        DrawText(
            FString::Printf(TEXT("HP: %.0f / %.0f"), Character->GetCurrentHealth(), Character->GetMaxHealth()),
            FLinearColor::White,
            40.0f,
            Y,
            nullptr,
            1.25f,
            false);
        Y += 28.0f;
    }

    if (const APvPArenaPlayerState* PlayerState = PC->GetPlayerState<APvPArenaPlayerState>())
    {
        DrawText(
            FString::Printf(TEXT("K / D: %d / %d"), PlayerState->GetKills(), PlayerState->GetDeaths()),
            FLinearColor::Yellow,
            40.0f,
            Y,
            nullptr,
            1.1f,
            false);
        Y += 28.0f;
    }

    if (const APvPArenaGameState* GameState = GetWorld() ? GetWorld()->GetGameState<APvPArenaGameState>() : nullptr)
    {
        DrawText(
            FString::Printf(TEXT("Time: %d"), GameState->GetRemainingRoundTimeSeconds()),
            FLinearColor::Green,
            40.0f,
            Y,
            nullptr,
            1.1f,
            false);
        Y += 28.0f;

        DrawText(
            FString::Printf(TEXT("Round: %s"), *RoundStateToText(static_cast<uint8>(GameState->GetRoundState()))),
            FLinearColor(0.0f, 1.0f, 1.0f, 1.0f),
            40.0f,
            Y,
            nullptr,
            1.1f,
            false);
    }
}

FString APvPArenaHUD::RoundStateToText(uint8 RoundStateValue) const
{
    switch (static_cast<EPvPARoundState>(RoundStateValue))
    {
    case EPvPARoundState::Playing:
        return TEXT("Playing");
    case EPvPARoundState::RoundEnd:
        return TEXT("RoundEnd");
    case EPvPARoundState::SuddenDeath:
        return TEXT("SuddenDeath");
    default:
        return TEXT("Unknown");
    }
}
