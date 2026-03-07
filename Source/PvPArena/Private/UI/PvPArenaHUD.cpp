#include "UI/PvPArenaHUD.h"

#include "Engine/Engine.h"
#include "Game/PvPArenaGameState.h"
#include "Game/PvPArenaPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
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
        Y += 28.0f;

        if (GameState->GetRoundState() == EPvPARoundState::RoundEnd)
        {
            DrawText(
                FString::Printf(TEXT("Result: %s"), *GetRoundResultText(PC, GameState)),
                FLinearColor::Yellow,
                40.0f,
                Y,
                nullptr,
                1.1f,
                false);
            Y += 28.0f;

            DrawText(
                FString::Printf(TEXT("Next Round In: %d"), GameState->GetRemainingRoundEndTimeSeconds()),
                FLinearColor(1.0f, 0.6f, 0.0f, 1.0f),
                40.0f,
                Y,
                nullptr,
                1.1f,
                false);
        }
    }
}

FString APvPArenaHUD::GetRoundResultText(APlayerController* PC, const APvPArenaGameState* GameState) const
{
    if (!PC || !GameState)
    {
        return TEXT("Unknown");
    }

    const APvPArenaPlayerState* LocalPlayerState = PC->GetPlayerState<APvPArenaPlayerState>();
    if (!LocalPlayerState)
    {
        return TEXT("Unknown");
    }

    const int32 LocalKills = LocalPlayerState->GetKills();
    int32 HighestOpponentKills = TNumericLimits<int32>::Min();
    bool bFoundOpponent = false;

    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        const APvPArenaPlayerState* PvPState = Cast<APvPArenaPlayerState>(PlayerState);
        if (!PvPState || PvPState == LocalPlayerState)
        {
            continue;
        }

        HighestOpponentKills = FMath::Max(HighestOpponentKills, PvPState->GetKills());
        bFoundOpponent = true;
    }

    if (!bFoundOpponent)
    {
        return TEXT("Pending");
    }

    if (LocalKills > HighestOpponentKills)
    {
        return TEXT("Win");
    }

    if (LocalKills < HighestOpponentKills)
    {
        return TEXT("Lose");
    }

    return TEXT("Draw");
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
