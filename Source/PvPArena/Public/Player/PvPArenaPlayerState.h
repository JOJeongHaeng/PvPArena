#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PvPArenaPlayerState.generated.h"

UCLASS()
class PVPARENA_API APvPArenaPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    APvPArenaPlayerState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category = "Player")
    void SetTeamId(int32 NewTeamId);

    UFUNCTION(BlueprintCallable, Category = "Player")
    void AddKill();

    UFUNCTION(BlueprintCallable, Category = "Player")
    void AddDeath();

    UFUNCTION(BlueprintCallable, Category = "Player")
    void SetDeadState(bool bNewIsDead, int32 NewRespawnRemainingSeconds);

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
    int32 TeamId;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
    int32 Kills;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
    int32 Deaths;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
    bool bIsDead;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
    int32 RespawnRemainingSeconds;
};
